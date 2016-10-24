#!/usr/bin/python
 
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time

# IP address of MQTT broker
hostMQTT='localhost'
# example of free MQTT broker:  'iot.eclipse.org'

clientId='myNameOfClient'

myTopic1='/domotique/garage/porte/'


# serial msg to arduino begin  with  prefAT and end with endOfLine
prefAT='AT+'
endOfLine='\n'
# arduino responds with those 2 kind of messages
msg2mqtt='2mq'
msg2py='2py'

devSerial='/dev/ttyUSB1'   # serial port the arduino is connected to
cmdSendValue='SendValue'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
topFromPy='py1/'
topFromOH='oh/'

# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg)

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

# read all available messages from arduino
def readArduinoAvailableMsg(seri):
	while seri.inWaiting():
		# because of readline function, dont forget to open with timeout
		response = seri.readline().replace('\n', '')
		#logp ("answer is:" + response, 'debug')
		tags = response.split(';')
		if tags[0] == msg2mqtt:
			# msg2mqtt: message to send to mqtt
			# I dont analyse those messages, I transmit to mqtt
			(topic, value) = tags[1].split(':')
			pyTopic = myTopic1 + topFromPy + topic
			# trace
			print('{} = {}'.format(pyTopic, value))
			mqttc.publish(pyTopic, value, retain=True)
		elif tags[0] == msg2py:
			# msg2py: message 2 python only
			# python use this to check connection with arduino
			print ('msg2py '+response)
		else :
			# I dont analyse, but I print
			print ('[garbage from '+devSerial+'] '+response)


mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect

#mqttc.connect('iot.eclipse.org', port=1883, keepalive=60, bind_address="")
cr = mqttc.connect(hostMQTT, port=1883, keepalive=60, bind_address="")
mqttc.loop_start()

# connection to arduino
# I use 9600, because I had many pb with pyserial at 38400 !!!
ser = serial.Serial(devSerial, baudrate=9600, timeout=0.2)
logp (str(ser), 'info')
# when we open serial to an arduino, this reset the board; it needs ~3s
time.sleep(0.2)
#I empty arduino serial buffer
response = ser.readline()
logp ("arduino buffer garbage: " + str(response), 'info')
time.sleep(3)

# loop to get connection to arduino


# infinite loop
# I regulary query a new measure value from arduino
# then I publish it
while True:
	time.sleep(sleepBetweenLoop)
	cmd = (prefAT + cmdSendValue + endOfLine).encode('utf-8')
	ser.write(cmd)
	logp (cmd)
	readArduinoAvailableMsg(ser)



