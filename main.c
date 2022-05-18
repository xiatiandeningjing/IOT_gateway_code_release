#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#if 0


int main()
{


	eventBase = initEvent();

	
	gatewayDevice.gatewayInitPoints(config);
	
	
	interface1.initInterfaceNode("http",jsonhttpConfigStr);
	interface1.device = gatewayDevice;
	
	interface2.initInterfaceNode("modbus",jsonmodbusConfigStr);
	interface2.device = gatewayDevice;


	interface1.setEventBase(eventBase);
	interface2.setEventBase(eventBase);

	
	interface1.onEvent("onmessage",interface1.recCb);
	interface2.onEvent("onmessage",interface2.recCb);

	
	gatewayDevice.addInterface(interface1);
	gatewayDevice.addInterface(interface2);

	gatewayDevice.run();

	

	/*
	task1 = addInterface("http",jsonConfig,topic);
	task2 = addInterface("modbus",jsonConfig,topic);
	
	taskList.add(task1);
	taskList.add(task2);
*/


	mainTaskRun(eventBase);
	
	
}


func addInterface(Interface,jsonConfig,topic);
{
	if (Interface == modbus)
	{
		Interface.name=modbus;
		Interface.topic=topic
		Interface.publish=modbus_publish;
		
		topicList.Add(Interface);
		modbus_init(jsonConfig,topic);
		
		return modbus_run;
	}
	if (Interface == http)
	{
		topicList.Add(modbus_publish,topic);
		http_init(jsonConfig,topic);
		return http_run;
	}


}

#endif



