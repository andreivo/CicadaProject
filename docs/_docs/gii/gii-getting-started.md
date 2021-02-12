---
title: Cicada Broker - Getting Started
tags: 
 - Project
 - Broker
description: Cicada Broker - Getting Started
---

# Cicada Broker - Getting Started

The Cicada Broker Project was developed to send data through the MQTT protocol. In this project, the RabbitMQ server was used as the basis for Cicada Broker.

RabbitMQ is an open-source messaging server (open source) developed in Erlang, implemented to support messages in the Advanced Message Queuing Protocol (AMQP) natively; also, it has an adapter for MQTT.  It makes it possible to deal with message traffic quickly and reliably, in addition to being compatible with several programming languages, having a native administration interface, and being multiplatform.

Among the applications of RabbitMQ are to make it possible to guarantee asynchronicity between applications, decrease the coupling between applications, distribute alerts, and control the queue of jobs in the background.

<p align="center">
	<img src="../../assets/img/CicadaBroker.png" align="center" height="auto" width="100%" style="max-width:600px">
<br><br><b>Fig 1: Cicada Broker</b>
</p>

The Cicada Broker Project has 2 components, the Messaging Server(3-RabbitMQ) and the Integration Mediator(4). Messaging Server(3-RabbitMQ) is responsible for receiving the message and Integration Mediator(4) is responsible for consuming messages from the Messaging Server(3-RabbitMQ) queue and persisting them in the database.

The Cicada Broker Project was started with the primary objective of subsidizing the development of Cicada DCP.
In this way, the Integration Mediator component was implemented in the form of a prototype and still needs new developments to make it robust enough to Cicada Project.
<b>NOTE: Use at your own risk!</b>








