---
title: Cicada DCP
tags: 
 - Project
 - DCP
description: Describes the Cicada DCP
---

# Cicada DCP

Cicada DCP is an equipment design with a low-cost Datalogger and Sensors integrated into a Data Collection Platform that has requirement high reliability and freedom for adaptations. The Cicada DCP was designed for native interoperability with the Cicada Broker, and all project is open-source and documented.

In practice, the Cicada DCP is a monoblock device with an embeddable electronic system that allows the integration of sensors for monitoring the environment and actuators, e.g., alarm sirens. Cicada DCP has humidity, temperature, and rainfall sensors by default, as shown in Fig. 1.

<p align="center">
	<img src="../../assets/img/CicadaDCP.png" align="center" height="auto" width="40%" style="max-width:250px">
	<img src="../../assets/img/CicadaDCPCut.png" align="center" height="auto" width="58%" style="max-width:350px">
<br><br><b>Fig 1: Cicada DCP Body Design</b>
</p>

<p align="center">
	<script>
		function getHeightRender() {
			width = window.innerWidth*0.95;
			height = width*0.67;
		   	return Math.min(420, height);
		}

		document.write("<iframe height=\'"+getHeightRender()+"\' width=\'95%\' frameborder=\'0\' src=\'https://render.githubusercontent.com/view/3d?url=https://raw.githubusercontent.com/andreivo/CicadaProject/main/structural/design/00-cicadadcp.stl\' title=\'00-cicadadcp.stl\'><\/iframe>")
	</script>
<br><br><b>Fig 2: Cicada DCP 3d View</b>
</p>


The body of the Cicada DCP was designed with a simple yet functional concept to serve the most varied audiences, which allows its construction through <b>3D printing</b> or industrial processes such as plastic injection.

{% include alert.html type="info" content="The entire project was developed as a case study of <b>Fault & INtegration-Driven System Engineering (FIND-SE)</b>, a methodological approach to fault-tolerant systems engineering (SE). The approach extends the knowledge of Model-Based System Engineering (MBSE). It is based on the principle of guiding the developer in a design concerned with failures, and especially those related to interoperability in the system integration process, which must be tolerated and treated. FIND-SE's great advantage is the proposal of a new paradigm based on guiding development through a principle, the same concept used in the TDD (Test Driven Development) and BDD (Behavior Driven Development) techniques." %}



