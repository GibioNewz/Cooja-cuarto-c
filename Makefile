CONTIKI_NG_PROJECT_NAME = sky-websense

PLATFORM ?= sky  

MODULES_REL += ./ 
PROJECT_SOURCEFILES += sky-websense.c


CONTIKI = ../..
include $(CONTIKI)/Makefile.include
