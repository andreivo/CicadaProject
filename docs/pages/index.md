---
layout: page
title: Cicada Project
permalink: /
---
<p align="center">
	<img src="assets/img/logo/CemadenLogColor.png" align="center" height="100" width="100">
</p>

<h1 align="center" style="color:#00055B;">Cicada Project</h1>


# Welcome

Welcome to the Cicada Project.

The Cicada Project is a collaboration between <a href="http://www.cemaden.gov.br/" target="_blank">Cemaden</a> and <a href="https://www.unifesp.br/" target="_blank">Unifesp</a> and was born out of a need by Cemaden.
Cemaden has an Observational network with more than 5000 Data Collection Platforms (DCPs) spread across the Brazilian territory. DCPs, in general, are imported equipment and require specialized labor; that is, it consumes a lot of financial resources to maintain its operation.

The Cicada Project aims to create a new management and operation model for the Cemaden observational network, based mainly on the decentralization of management authority under the DCPs that make up the network. This new concept creates a collaborative model that promotes the development of a private network of Cicada DCPs. To enable this decentralization, DCPs must follow a standard model and have the lowest possible cost of acquisition and operation, resulting in a significant reduction in the cost of maintenance and encouraging the expansion of the network.

<p align="center">
	<img src="assets/img/New_CONOPS_CEMADEN.jpg" align="center" height="auto" width="70%">
<br><br><b>Fig 1: Cicada CONOPs</b>
</p>



## The Project

The project was divided into 3 groups to structure its development and allow research to be developed in a segmented way. Figure 2 shows the division of the groups.

<p align="center">
	<img src="assets/img/CicadaProjectParts.png" align="center" height="auto" width="55%" max-width="725px">
<br><br><b>Fig 2: Cicada Project Groups</b>
</p>

### Group I (Cicada DCP):

Design of a low cost Datalogger and Sensors, integrated in a Data Collection Platform (Cicada DCP) with high reliability and freedom for adaptations.
The Cicada DCP will be designed for native interoperability with the Cicada Broker.
Fully documented and open-source hardware.

What are these features? You should see the {% include doc.html name="Cicada DCP" path="docs/gi/gi-index" %}

### Group II (Cicada Broker):

An IoT Broker with a standard protocol for sending and receiving environmental data through a reliable communication.

### Group III (Cicada Data Science):

A set of algorithms for Data Science (Cicada DS) to produce stats and knowledge about disaster risks.
In addiction, the environmental information should be accessed in a web platform (Cicada Web) and in a mobile app (Cicada Mobile).


## Features

What are these features? You should see the {% include doc.html name="Getting Started" path="getting-started" %}
guide for a complete summary. Briefly:

 - *User interaction* including consistent permalinks, links to ask questions via GitHub issues, and edit the file on GitHub directly.
 - *Search* across posts, documentation, and other site pages, with an ability to exclude from search.
 - *External Search* meaning an ability to link any page tag to trigger an external search.
 - *Documentation* A documentation collection that was easy to organize on the filesystem, render with nested headings for the user, and refer to in markdown.
 - *Pages* A separate folder for more traditional pages (e.g, about).
 - *Navigation*: Control over the main navigation on the left of the page, and automatic generation of table of contents for each page on the right.
 - *News* A posts feed for news and updates, along with an archive (organized by year).
 - *Templates* or specifically, "includes" that make it easy to create an alert, documentation link, or other content.
 - *Continuous Integration* recipes to preview the site


For features, getting started with development, see the {% include doc.html name="Getting Started" path="getting-started" %} page. Would you like to request a feature or contribute?
[Open an issue]({{ site.repo }}/issues)
