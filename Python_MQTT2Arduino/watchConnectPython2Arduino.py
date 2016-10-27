#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import sys, getopt

# IP address of MQTT broker
hostMQTT='localhost'
# example of free MQTT broker:  'iot.eclipse.org'

clientId='myNameOfClient'

logDir='/media/ramdisk/openhab/logPython/'
logfile=watchPyMaster.log
logStartTime=0.

myTopic1='/domotique/garage/topictest/'
myTopic2='/domotique/garage/topictest2/'

devSerial='/dev/ttyUSB1'   # serial port the arduino is connected to
cmdSendValue='SendValue'


# serial msg to arduino begin  with  prefAT / prefDO and end with endOfLine
prefAT='AT+'
prefDO='DO+'
endOfLine='\n'
# arduino responds with those 2 kind of messages
msg2py='2py'
msg2mqtt='2mq'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
namePy='pym'
topFromPy= namePy + '/'
topFromOH='oh/'
topFromSys='sys/'


# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg, file=logfile)

# log file must not grow big
# I need to overwrite it often
def reOpenLogfile(logfileName):
	global logStartTime, logfile
	#
	if logfileName != '' :
		try:
			# I close file if needed
			if ( not logfile.closed) and (logfile.name != '<stdout>') :
				logfile.close()
			# file will be overwritten
			if (logfileName != '<stdout>') :
				logfile = open(logfileName, "w", 1)
			logStartTime = time.time()
			logp('logStartTime:' + time.asctime(time.localtime(time.time())), 'info')
		except IOError:
			print('[error] could not open logfile:' + logfileName)
			sys.exit(3)
	else :
		print('I cant re open logfile. name is empty')


def read_args(argv):
	# optional args have default values above
	global logfile, hostMQTT, namePy, myTopic1, myTopic2, devSerial
	logfileName = ''
	try:
		opts, args = getopt.getopt(argv,"hl:b:n:t:u:d:",["logfile=","broker=","namepy=","mytopic1=","mytopic2=","devserial="])
	except getopt.GetoptError:
		print ('serial2MQTTduplex.py -l <logfile> -n <namepy> -t <mytopic1> -u <mytopic2> -d <devserial>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print ('serial2MQTTduplex.py -l <logfile> -n <namepy> -t <mytopic1> -u <mytopic2> -d <devserial>')
			sys.exit()
		elif opt in ("-l", "--logfile"):
			logfileName = arg
		elif opt in ("-b", "--broker"):
			hostMQTT = arg
		elif opt in ("-n", "--namepy"):
			namepy = arg
		elif opt in ("-t", "--mytopic1"):
			myTopic1 = arg
		elif opt in ("-u", "--mytopic2"):
			myTopic2 = arg
		elif opt in ("-d", "--devserial"):
			devSerial = arg
	logp('logfile is '+ logfileName, 'debug')
	logp('broker is '+ hostMQTT, 'debug')
	logp('namepy is '+ namePy, 'debug')
	logp('mytopic1 is '+ myTopic1, 'debug')
	logp('mytopic2 is '+ myTopic2, 'debug')
	logp('devserial is '+ devSerial, 'debug')
	# I try to open logfile
	if logfileName != '' :
		reOpenLogfile(logfileName)


logStartTime = time.time()
if __name__ == "__main__":
	read_args(sys.argv[1:])


# if logfile is old, we remove it and overwrite it
#   because it must not grow big !
def checkLogfileSize(logfile):
	global logStartTime
	if (time.time() - logStartTime) > 30:
		#print('reOpenLogfile of name:' + logfile.name)
		reOpenLogfile(logfile.name)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
	logp("Connected with result code "+str(rc), 'info')
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	mqttc.subscribe(myTopicSys +'#')
	mqttc.message_callback_add(myTopicSys+ '#', on_message_myTopicSys)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
	logp('msg:' +msg.topic+" : "+str(msg.payload), 'garbage')

# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_myTopicSys(client, userdata, msg):
	logp("spec callbackSys,"+msg.topic+":"+str(msg.payload), 'info')

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
			logp('{} = {}'.format(pyTopic, value), 'garbage')
		elif tags[0] == msg2py:
			# msg2py: message 2 python only
			# python use this to check connection with arduino
			logp ('msg2py '+response, 'info')
		else :
			# I dont analyse, but I print
			logp (response, 'unknown from '+devSerial)



mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect

cr = mqttc.connect(hostMQTT, port=1883, keepalive=60, bind_address="")
mqttc.loop_start()

# connection to arduino
# I use 9600, because I had many pb with pyserial at 38400 !!!
#ser = serial.Serial(devSerial, baudrate=9600, timeout=0.2)
logp (str(ser), 'info')
# when we open serial to an arduino, this reset the board; it needs ~3s
time.sleep(0.2)
#I empty arduino serial buffer
response = ser.readline()
logp ("arduino buffer garbage: " + str(response), 'info')
time.sleep(2)

# loop to get connection to arduino


# infinite loop
# I regulary check new serial devices /dev/ttyU* /dev/ttyA*
# when I find one, I try to link arduino to MQTT
while True:
	time.sleep(sleepBetweenLoop)
	#  TO DO
	readArduinoAvailableMsg(ser)
	#check size
	checkLogfileSize(logfile)



