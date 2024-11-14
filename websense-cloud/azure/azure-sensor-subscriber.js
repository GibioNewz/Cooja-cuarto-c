'use strict';

var EventHubClient = require('azure-event-hubs').Client;

var connectionString = 'Endpoint=sb://ihsuprodcqres017dednamespace.servicebus.windows.net/;SharedAccessKeyName=iothubowner;SharedAccessKey=HNLKYUkuGlRvBkudCNUmYjgeuSAMhpgVpAIoTM9vuKI=;EntityPath=iothub-ehub-test-tel32-56940684-e97b87dd9e';

var printError = function (err) {
    console.log("Error: ", err.message);
};
  
var printMessage = function (message) {
    console.log('Message received: ');
    console.log(JSON.stringify(message.body));
    console.log('');
};

var client = EventHubClient.fromConnectionString(connectionString);
client.open()
    .then(client.getPartitionIds.bind(client))
    .then(function (partitionIds) {
        return partitionIds.map(function (partitionId) {
            return client.createReceiver('$Default', partitionId, { 'startAfterTime' : Date.now() - 10 * 60 * 1000 }).then(function(receiver) {
                console.log('Created partition receiver: ' + partitionId);
                receiver.on('errorReceived', printError);
                receiver.on('message', function (message) {
                    console.log('Partition ' + partitionId + ' received a message.');
                    printMessage(message);
                });
            });
        });
    })
    .catch(printError);

