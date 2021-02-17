---
title: Cicada Broker
tags: 
 - Project
 - Broker
description: Describes the Cicada Broker
---

# Cicada Broker

The Cicada Broker Project was developed to send data through the MQTT protocol. In this project, the RabbitMQ server was used as the basis for Cicada Broker.

RabbitMQ is an open-source messaging server (open source) developed in Erlang, implemented to support messages in the Advanced Message Queuing Protocol (AMQP) natively; also, it has an adapter for MQTT.  It makes it possible to deal with message traffic quickly and reliably, in addition to being compatible with several programming languages, having a native administration interface, and being multiplatform.

Among the applications of RabbitMQ are to make it possible to guarantee asynchronicity between applications, decrease the coupling between applications, distribute alerts, and control the queue of jobs in the background.

<p align="center">
	<img src="../../assets/img/CicadaBroker.png" align="center" height="auto" width="100%" style="max-width:600px">
<br><br><b>Fig 1: Cicada Broker</b>
</p>

The Cicada Broker Project has 2 components, the Messaging Server(3-RabbitMQ) and the Integration Mediator(4). Messaging Server(3-RabbitMQ) is responsible for receiving the message and Integration Mediator(4) is responsible for consuming messages from the Messaging Server(3-RabbitMQ) queue and persisting them in the database.

{% include alert.html type="warning" content="The Cicada Broker Project was started with the primary objective of subsidizing the development of Cicada DCP.
In this way, the Integration Mediator component was implemented in the form of a prototype and still needs new developments to make it robust enough to Cicada Project.<br><br>
<b>NOTE: Use at your own risk!</b>" %}

### <b>Working in progress</b>

You would like to collaborate with the project? Please contact our team in {% include doc.html name="About" path="../about" %}.

## Messaging Server

We strongly recommend using RabbitMQ as the messaging server. Remember that RabbitMQ uses AMQP as the standard protocol. For it to work correctly with the Cicada DCP, the MQTT plugin must be active.

There are several commercial messaging servers based on RabbitMQ, such as <a href="https://www.cloudamqp.com/" target="_blank">CloudAMPQ</a>. 

If you choose to install your own RabbitMQ message server, see the official website <a href="https://www.rabbitmq.com/" target="_blank">www.rabbitmq.com</a>.

To install, see <a href="https://www.rabbitmq.com/download.html" target="_blank">www.rabbitmq.com/download.html</a>.

To activate the MQTT plugin, see <a href="https://www.rabbitmq.com/mqtt.html" target="_blank">www.rabbitmq.com/mqtt.html</a>.

## Integration Mediator

The Integration Mediator's main objective is to consume the data in the message server queue and translate the communication between Cicada DCP to perform the recording in the Database standard.
The first version in the form of a prototype was developed in Java, and its source code can be found in our repositories <a href="https://github.com/andreivo/CicadaProject/tree/main/brokerMediator" target="_blank">GitHub</a>.

If you want to use this integration mediator, download the source code and create the file <b>"./brokerMediator/src/main/java/com/fibase/QueueIdentity.java"</b>.
The QueueIdentity.java file contains the identities of access to the Message Servers, which must follow the following format:

```java
public class QueueIdentity {
    //Identification of the message server that will be consumed. 
    //The keys are in the RabbitMQ standard. 
    public static String QUEUEUSERNAME = "username";
    public static String QUEUEVIRTUALHOST = "virtualhost";
    public static String QUEUEPASSWORD = "passwd";
    public static String QUEUEHOST = "hostname";
    public static String QUEUE_NAME = "queuename";
    public static String EXCHANGE_NAME = "exchangename";
    public static String ROUTINGKEY = "routingkey";

    //IoT server identification to simulating messages 
    // replication in another gateway. 
    //This service is optional and used for the publish on an MQTT 
    // server prepared to display on a Dashboard, like Ubidots.com.
    public static String MQTTTOKEN = "mqtttoken";
    public static String MQTTBROKERURL = "mqtthost";
    public static String MQTTTOPIC = "topic";
}
``` 

### Ontologies use

This project was developed based on the system engineering approaches proposed by FIND-SE, which presents semantic integration techniques for heterogeneous systems.

The observational network formed by the Cicada Project is firmly based on the composition of heterogeneous systems with operational independence. It is in this context that semantic conflicts or the difficulties that arise from this fact may occur. Semantic conflicts occur when systems use different meanings for the same elements: when the elements appear to have the same meaning but do not.

To solve this fact, the Mediator's project was built based on ontologies to define semantic mappings.

An ontology was built in the mediator's project using the <a href="https://protege.stanford.edu/" target="_blank">Protégé</a> software and found on our <a href="https://github.com/andreivo/CicadaProject/tree/main/brokerMediator/ontologies" target="_blank">GitHub</a>.

Do not forget to update the path where the ontology can be found in the <b>"./brokerMediator/src/main/java/com/fibase/Constants.java"</b> file, as follows:

```java
public class Constants {
    public static final String ONTOLOGYPATH = "../ontologies/OR_OBSNetwork_NewArchitecture.rdf";
    public static final String DATAPATH = "../DATA/";
}
``` 

{% include alert.html type="warning" content="DATAPATH refers to the path where the files will be saved in CSV format with the messages that will be received. It is a simplified simulation of data persistence." %}

## Message Format

The messages exchanged between Cicada DCP and the Integration Mediator follow a JSON pattern of key-value. The following pattern shows an example of an exchanged message:

```json
{
  "tokenStation": "22222222F",
  "passwdStation": "PASSWD",
  "sentDateTime": "2020-12-09 18:01:56Z",
  "measures": [
    {
      "sensorExternalCode": 20,
      "dataType": "pluv",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "8"
    },
    {
      "sensorExternalCode": 21,
      "dataType": "vin",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "12.58"
    },
    {
      "sensorExternalCode": 22,
      "dataType": "vso",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "12.58"
    },
    {
      "sensorExternalCode": 23,
      "dataType": "temp",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "24"
    },
    {
      "sensorExternalCode": 24,
      "dataType": "hum",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "70"
    }
  ],
  "metadata": [
    {
      "dataType": "la",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "-23.7"
    },
    {
      "dataType": "lo",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "-44.7"
    },
    {
      "dataType": "bkt",
      "collectDateTime": "2020-12-09 18:01:56Z",
      "dataValue": "3.14"
    },
    {
      "dataType": "cqs",
      "collectDateTime": "2021-02-17 12:10:00Z",
      "dataValue": "100",
      "context": "{'cty':'WIFI','icc':'','lip':'192.168.0.160'}"
    },
    {
      "dataType": "fmw",
      "collectDateTime": "2021-02-17 12:10:00Z",
      "dataValue": "0.0.1-alpha"
    },
    {
      "dataType": "dfmw",
      "collectDateTime": "2021-02-17 12:10:00Z",
      "dataValue": "2021-01-01T10:10:00Z"
    }
  ]
}
```
The message is divided into 3 parts; the first is the body of the message, followed by a collection of measures, and finally, a collection of metadata. 

### <ins>Message Body</ins>

The description of the Message Body fields is described below:

| Key-field      | Required | Description   |
| :---           | :---:    | :---          |
| tokenStation   | *        | Represents the unique DCP's identification. |
| passwdStation  | *        | Represents the DCP's password to access AP wizard DCP and to the Integration Mediator accept data published. |
| sentDateTime   | *        | Represents the date and time that DCP published the data. |
| measures       | **       | Represents the **measures** collection of the various sensors. This field is required but can be suppressed if the field "metadata" is present. |
| metadata       | **       | Represents the **metadata** collection of the system. This field is required but can be suppressed if the field "measures" is present. |
 
### <ins>Measures</ins>

The message body "measures" field also follows the JSON key-value standard. The following table shows the meanings of the fields of the "measures" section.

| Key-field          | Required | Description   |
| :---               | :---:    | :---          |
| sensorExternalCode | *        | Represents the unique Sensor code identification. |
| dataType           | *        | Represents the Sensor data type. |
| collectDateTime    | *        | Represents the date and time that Sensor data was collected. |
| dataValue          | *        | Represents the data value was collected. |

### <ins>Metadata</ins>

The message body "metadata" field also follows the JSON key-value standard. The following table shows the meanings of the fields of the "metadata" section.

| Key-field          | Required | Description   |
| :---               | :---:    | :---          |
| dataType           | *        | Represents the system metadata data type. |
| collectDateTime    | *        | Represents the date and time that metadata was collected. |
| dataValue          | *        | Represents the metadata value was collected. |
| context            |          | Represents a string to include general information about metadata. It is strongly recommended that this field is also filled with the key-value standard in JSON format. |

### Equivalency Ontologic

The Integration Mediator uses ontological equivalences keys in the JSON message to reduce the traffic size. 
Cicada DCP uses the renamed keys when generating the messages that will be sent. Integration Mediator consumes messages and translates them by assessing ontological equivalences so that they can be understood and persisted in the database.

The equivalences are presented below: 

| Key-field renamed  | Message part  | Key-field          | 
| :---               | :---          | :---               |
| tknDCP             | Message Body  | tokenStation       |
| pwdDCP             | Message Body  | passwdStation      |
| sntDT              | Message Body  | sentDateTime       |
| snsEC              | Measures      | sensorExternalCode |
| dtT                | Measures      | dataType           |
| colDT              | Measures      | collectDateTime    |
| val                | Measures      | dataValue          |
| dtT                | Metadata      | dataType           |
| colDT              | Metadata      | collectDateTime    |
| val                | Metadata      | dataType           |

{% include alert.html type="info" content="It is worth mentioning that all equivalences are provided in the ontology file available on GitHub." %}

