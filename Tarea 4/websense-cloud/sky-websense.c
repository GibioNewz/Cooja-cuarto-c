
#include "contiki.h"
#include "net/ipv6/uip.h"
#include "os/net/app-layer/httpd-simple/httpd-simple.h"
#include "dev/sht3x-sensor.h" // Sensor SHT3x para temperatura
#include "dev/light-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

PROCESS(web_sense_process, "Sense Web Demo");
PROCESS(webserver_nogui_process, "Web server");
AUTOSTART_PROCESSES(&web_sense_process, &webserver_nogui_process);

#define HISTORY 16
static int temperature[HISTORY];
static int light1[HISTORY];
static int sensors_pos;

/*---------------------------------------------------------------------------*/
static int
get_light(void)
{
  return 10 * light_sensor.value(0) / 7;  
}
/*---------------------------------------------------------------------------*/
static int
get_temp(void)
{
  return sht3x_sensor.value(SHT3X_SENSOR_TEMP) / 100; 
}
/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Contiki-NG Web Sense</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
/*---------------------------------------------------------------------------*/
/* Solo una solicitud a la vez */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
static void
generate_chart(const char *title, const char *unit, int min, int max, int *values)
{
  int i;
  blen = 0;
  ADD("<h1>%s</h1>\n"
      "<img src=\"http://chart.apis.google.com/chart?"
      "cht=lc&chs=400x300&chxt=x,x,y,y&chxp=1,50|3,50&"
      "chxr=2,%d,%d|0,0,30&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max, min, max, unit);
  for(i = 0; i < HISTORY; i++) {
    ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
  }
  ADD("\">");
}
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  if(strncmp(s->filename, "/index", 6) == 0 ||
     s->filename[1] == '\0') {
    blen = 0;
    ADD("<h1>Lecturas actuales</h1>\n"
        "Luz: %u<br>"
        "Temperatura: %u&deg; C",
        get_light(), get_temp());
    SEND_STRING(&s->sout, buf);

  } else if(s->filename[1] == '0') {
    /* Apagar leds */
    leds_off(LEDS_ALL);
    SEND_STRING(&s->sout, "¡Leds apagados!");

  } else if(s->filename[1] == '1') {
    /* Encender leds */
    leds_on(LEDS_ALL);
    SEND_STRING(&s->sout, "¡Leds encendidos!");

  } else {
    if(s->filename[1] != 't') {
      generate_chart("Luz", "Luz", 0, 500, light1);
      SEND_STRING(&s->sout, buf);
    }
    if(s->filename[1] != 'l') {
      generate_chart("Temperatura", "Celsius", 15, 50, temperature);
      SEND_STRING(&s->sout, buf);
    }
  }

  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return send_values;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  sensors_pos = 0;

  etimer_set(&timer, CLOCK_SECOND * 2);
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht3x_sensor);  // Activa el sensor de temperatura

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    light1[sensors_pos] = get_light();
    temperature[sensors_pos] = get_temp();
    sensors_pos = (sensors_pos + 1) % HISTORY;
  }

  PROCESS_END();
}

