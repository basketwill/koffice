<?xml version="1.0"?>
<KivioShapeStencil creator="kate">
		<KivioSMLStencilSpawnerInfo>
			<Author data="Ian Reinhart Geiser"/>
			<Title data="Xor"/>
			<Id data="Xor"/>
			<Description data="Logical Xor Gate"/>
			<Version data="0.1"/>
			<Web data="www.kde.org"/>
			<Email data="geiseri@kde.org"/>
			<Copyright data="Copyright (C) 2002 Ian Reinhart Geiser"/>
			<AutoUpdate data="off"/>
		</KivioSMLStencilSpawnerInfo>

	<KivioConnectorTarget  x="20" y="0"/>
	<KivioConnectorTarget  x="10" y="54"/>
	<KivioConnectorTarget  x="20" y="52.5"/>
	<KivioConnectorTarget  x="30" y="54"/>

	<Dimensions w="40" h="60" defaultAspect="1"/>
	<KivioShape type="ClosedPath" name="Or">
		<KivioPoint x="20" y="0" type="bezier"/>   
		<KivioPoint x="40" y="10" type="bezier"/>  
		<KivioPoint x="40" y="30" type="bezier"/>  
		<KivioPoint x="40" y="50" type="bezier"/>  

		<KivioPoint x="40" y="50" type="bezier"/>  
		<KivioPoint x="30" y="40" type="bezier"/>  
		<KivioPoint x="10" y="40" type="bezier"/>  
		<KivioPoint x="0" y="50" type="bezier"/>  

		<KivioPoint x="0" y="50" type="bezier"/>  
		<KivioPoint x="0" y="30" type="bezier"/>  
		<KivioPoint x="0" y="10" type="bezier"/>  
		<KivioPoint x="20" y="00" type="bezier"/> 
	</KivioShape>

	<KivioShape type="OpenPath" name="X">
		<KivioPoint x="0" y="60" type="bezier"/>  
		<KivioPoint x="10" y="50" type="bezier"/> 
		<KivioPoint x="30" y="50" type="bezier"/> 
		<KivioPoint x="40" y="60" type="bezier"/>
	</KivioShape>
	<KivioShape type="TextBox" name="TextBox0"  x="0.0" y="0.0" w="40" h="60" />
</KivioShapeStencil>