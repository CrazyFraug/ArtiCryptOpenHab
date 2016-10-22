#!/usr/bin/python
 
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time

# IP address of MQTT broker
hostMQTT='localhost'
# example of free MQTT broker:  'iot.eclipse.org'

clientId='myNameOfClient'

myTopic1='oh/living/#'

# serial msg to arduino begin  with  prefAT and end with endOfLine
prefAT='AT+'
endOfLine='\n'

devSerial='/dev/ttyUSB1'   # serial port the arduino is connected to
cmdSendValue='SendValue'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
topicPrefPy='py1/'


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
	print("Connected with result code "+str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	mqttc.subscribe(myTopic1)
	mqttc.message_callback_add(myTopic1, on_message_myTopic1)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
	print(msg.topic+" "+str(msg.payload))
	cmd2arduino = prefAT + str(msg.payload)
	ser.write(cmd2arduino)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message_myTopic1(client, userdata, msg):
	print("spec callback1,"+msg.topic+" "+str(msg.payload))
	cmd2arduino = prefAT + str(msg.payload)
	ser.write(cmd2arduino)


ser = serial.Serial(devSerial, 57600, timeout=1)
mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect

#mqttc.connect('iot.eclipse.org', port=1883, keepalive=60, bind_address="")
cr = mqttc.connect(hostMQTT, port=1883, keepalive=60, bind_address="")
mqttc.loop_start()

#I empty arduino serial buffer
ser.write("s\n");
time.sleep(1)
ser.write("s\n");

while True:
	time.sleep(sleepBetweenLoop)
	ser.write(prefAT + cmdSendValue + endOfLine)
	print prefAT + cmdSendValue + endOfLine
	response = ser.readline().replace(endOfLine, '')
	print "answer is:" + response
	tags = response.split(';')
	if tags[0] == 'm2p':
		(topic, value) = tags[1].split(':')
		pyTopic = topicPrefPy + topic
		# trace
		print('{} = {}'.format(pyTopic, value))
		mqttc.publish(pyTopic, value, retain=True)


