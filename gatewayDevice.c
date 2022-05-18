#include "gatewayEnv.h"
#include "httpclient.h"
#include "stream.h"
#include "iot_mqttclient.h"

void mqttOnResponse(void *str)
{	
	iot_debugx_("%s temperature value %d\n",__FUNCTION__,atoi(str));
}


int main(void)
{	
	int ret = -1;
	char mqttconfig[] = "{\"topic\":\"/xnn/topic\",\"clientId\":\"xnn_client\",\
		 \"username\":\"\",\"password\":\"\",\
		 \"host\":\"127.0.0.1\",\"port\":\"1883\"}";

	ret = mqttinitInterfaceNode(mqttconfig,mqttOnResponse);

	if (0 == ret)
		 mqttinterfaceDispatch();
	else
		return -1;

	
	if (0 == ret)
		ret = mqttmakeRequest();
	else
		return -1;


	while(1)
	{
		sleep(1);
	}

}

