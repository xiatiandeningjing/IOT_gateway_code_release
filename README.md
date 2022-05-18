
# IOT_gateway_code
 
#版权使用说明
工程由本人独立完成，目的在简化物联网行业不同服务的扩展接口，将扩展接口标准化。


#机制概括
+ 事件驱动
+ stream传输


# demo编译
make



# demo使用
+预先安装mqtt服务器， 可以使用开源工程mosquitto模拟
+预先安装一个mqtt客户端模拟器，可以使用mqtt.fx,用来与xnn_gateway通信转发
++ make
++ ./xnn_gateway  

++ mqtt连接配置：
"{\"topic\":\"/xnn/topic\",\"clientId\":\"xnn_client\",\
\"username\":\"\",\"password\":\"\",\
\"host\":\"127.0.0.1\",\"port\":\"1883\"}";



# 接口说明
+程序主文件
	++gatewayDevice.c
	
+paho.mqtt.embedded 工程源码修改点
	++socket传输层接口替换
		++mqttioRecv替换linux_read，工程中网络传输层用stream组件替换
		++event接口替换select事件接口
	++主线程与mqtt通信用事件接口


+模块间通信事件接口
	++event_on 设置监听事件
	++event_emit触发事件
