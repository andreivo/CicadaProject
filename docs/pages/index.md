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

The Cicada Project is a broad concept that can create the new management and operation model for the Cemaden observational network, based mainly on the decentralization of management authority under the DCPs that make up the network. This new concept creates a collaborative model that promotes the development of a private network of Cicada DCPs. To enable this decentralization, DCPs must follow a standard model and have the lowest possible cost of acquisition and operation, resulting in a significant reduction in the cost of maintenance and encouraging the expansion of the network.

<p align="center">
	<img src="assets/img/New_CONOPS_CEMADEN.jpg" align="center" height="auto" width="70%">
<br><br><b>Fig 1: Cicada CONOPs</b>
</p>



## The Project

The project was divided into 3 groups to structure its development and allow research to be developed in a segmented way. Figure 2 shows the division of the groups.

<p align="center">
	<img src="assets/img/CicadaProjectParts.png" align="center" height="auto" width="60%" max-width="725px">
<br><br><b>Fig 2: Cicada Project Groups</b>
</p>

### <ins>Group I (Cicada DCP):</ins>

Cicada DCP is an equipment design with a low-cost Datalogger and Sensors integrated into a Data Collection Platform that has requirement high reliability and freedom for adaptations. The Cicada DCP was designed for native interoperability with the Cicada Broker, and all project is open-source and documented.

In practice, the Cicada DCP is a monoblock device with a built-in electronic system that allows the integration of sensors for monitoring the environment and actuators, e.g., alarm sirens. By default, Cicada DCP has humidity, temperature, and rainfall sensors, as shown in Fig. 3.

<p align="center">
	<img src="assets/img/CicadaDCP.png" align="center" height="auto" width="25%" max-width="200px">
	<img src="assets/img/CicadaDCPCut.png" align="center" height="auto" width="35%" max-width="200px">
<br><br><b>Fig 3: Cicada DCP Body Design</b>
</p>

<p align="center">
	<script>
		function getHeightRender() {
			width = window.innerWidth*0.95;
			height = width*0.67;
		   	return Math.min(420, height);
		}

		document.write("<iframe height=\'"+getHeightRender()+"\' width=\'95%\' frameborder=\'0\' src=\'https://render.githubusercontent.com/view/3d?url=https://raw.githubusercontent.com/andreivo/CicadaProject/main/mechanical/design/00-cicadadcp.stl\' title=\'00-cicadadcp.stl\'><\/iframe>")
	</script>
<br><br><b>Fig 4: Cicada DCP 3d View</b>
</p>


The body of the Cicada DCP was designed with a simple yet functional concept to serve the most varied audiences, which allows its construction through <b>3D printing</b> or industrial processes such as plastic injection.

The entire project was developed as a case study of <b>Fault & INtegration-Driven System Engineering (FIND-SE)</b>, a methodological approach to fault-tolerant systems engineering (SE). The approach extends the knowledge of Model-Based System Engineering (MBSE). It is based on the principle of guiding the developer in a design concerned with failures, and especially those related to interoperability in the system integration process, which must be tolerated and treated. FIND-SE's great advantage is the proposal of a new paradigm based on guiding development through a principle, the same concept used in the TDD (Test Driven Development) and BDD (Behavior Driven Development) techniques.

For more details, see the section {% include doc.html name="Cicada DCP" path="gi/gi-index" %}.

### <ins>Group II (Cicada Broker):</ins>

The project was developed to send data through the MQTT protocol. In this project, the RabbitMQ server was used as the basis for Cicada Broker.
Cicada Broker was also developed using FIND-SE's best practices and proposed a standard protocol for sending and receiving environmental data. 

<p align="center">
	<img src="assets/img/CicadaBroker.png" align="center" height="auto" width="75%" max-width="600px">
<br><br><b>Fig 5: Cicada Broker</b>
</p>

For more details, see the section {% include doc.html name="Cicada Broker" path="gii/gii-index" %}.

### <ins>Group III (Cicada Data Science):</ins>

A set of algorithms for Data Science (Cicada DS) to produce stats and knowledge about disaster risks.
In addiction, the environmental information should be accessed in a web platform (Cicada Web) and in a mobile app (Cicada Mobile).

For more details, see the section {% include doc.html name="Cicada Data Science" path="giii/giii-index" %}.

<b>Not started yet.</b> You would like to collaborate with the project; please contact us. For more details, contact our team in {% include doc.html name="About" path="../about" %}.


## Our Team

For more details about our Team, see the section {% include doc.html name="About" path="../about" %}.
