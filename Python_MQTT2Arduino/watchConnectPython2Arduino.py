#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import sys, getopt
import ast
import os, glob


fileNameListSketch = 'listSketch.txt'
repTmp='/media/ramdisk/openhab/logPython'
repTmp='/home/arnaud/Workspaces/Raspberry/openhab-200/conf/Python_MQTT2Arduino'
devSearchString='/dev/tty[UA]*'
sleepBetweenLoop=2

prefAT='AT+'
cmdIdSketch='idSketch'
endOfLine='\n'
msg2py='2py'

logfile=sys.stdout

startedSerial2MQTT = {}
lastListDev = []
listActiveDev = []

# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg, file=logfile)


def readListSketchFromFile(fileName):
	"read filename and append valid dict lines in returned dict"
	logp('reading list of arguments for sketch in file : ' + fileName)
	try:
		fileListSketch = open(fileName, 'r')
	except:
		logp('could not open list of sketch configuration file: ' + fileName, 'error')
		return {}
	#
	strLines = fileListSketch.readlines()
	fileListSketch.close()
	dSketch = {}
	for strl in strLines:
		try:
			itemDict = ast.literal_eval(strl)
		except:
		  logp('line fails as dict:' + strl)
		else:
			if (type(itemDict) == type({})):
				dSketch.update(itemDict)
			else:
				logp ('line NOT dict:' + strl)
	fileListSketch.close()
	return dSketch

def launchSerial2MQTT(da, arepTmp,adevSerial):
	"Launches the python script serial2MQTTduplex.py thanks to the dictionnary of arg"
	# test that it has all required arg
	for keyNeeded in ("logfile","broker","namepy","mytopic1","mytopic2"):
		if not (da.has_key(keyNeeded)):
			logp('I refuse to launch serial2MQTTduplex ...', 'error')
			logp(' because I cannot find key ' + keyNeeded, 'error')
			return
	cmd = 'serial2MQTTduplex.py'+ ' -t '+da['mytopic1'] + ' -n '+da['namepy'] + ' -l '+arepTmp+'/' + da['logfile'] + ' -b '+da['broker'] + ' -d ' + adevSerial +' &'
	os.system(cmd)


def askIdSketchSerial(adevSerial):
	"try to know the id of the arduino sketch linked on serial adevSerial"
	logp('trying to recognize arduino sketch on serial:' + adevSerial , 'info')
	rIdSketch = ''
	try:
		ser = serial.Serial(adevSerial, baudrate=9600, timeout=0.2)
		logp (str(ser), 'info')
		time.sleep(0.2)
		#
		#I empty arduino serial buffer
		response = ser.readline()
		logp ("arduino buffer garbage: " + str(response), 'info')
		#
		# when we open serial to an arduino, this reset the board; it needs ~3s
		time.sleep(3)
	except:
			logp ('could not ask idSketch to serial '+adevSerial , 'warning')
			return ''
	#
	# I send a sys cmd to ask for id
	cmd = (prefAT + cmdIdSketch + endOfLine)
	ser.write(cmd)
	logp (cmd)
	# I give time to respond
	time.sleep(0.2)
	# I sort the response, I will get rid of unused messages
	while ser.inWaiting():
		# because of readline function, dont forget to open with timeout
		response = ser.readline().replace('\n', '')
		#logp ("answer is:" + response, 'debug')
		tags = response.split(';')
		if tags[0] == msg2py:
			# msg2py: message 2 python only
			# python use this to check connection with arduino
			logp ('msg2py '+response, 'info')
			nameInd = response.rfind(':')	
			if (nameInd >= 0):
				rIdSketch = response[nameInd+1:]
				return rIdSketch
		else :
			# I dont analyse, but I print
			logp (response, 'unknown from '+adevSerial)

def giveUpdateListSerialDev(oldListDev, genericLsNameDev):
	"list all serial dev connected; updates oldListDev, return list of new dev and dead dev"
	newListDev = glob.glob(genericLsNameDev)
	newListDev.sort()
	lDevNew = []; lDevDead = []
	if (cmp(oldListDev,newListDev) != 0):
		# I look for new connected device
		for dev in newListDev:
			if oldListDev.count(dev) == 0:
				lDevNew.append(dev)
		# I look for dead (not) connected device
		for dev in oldListDev:
			if newListDev.count(dev) == 0:
				lDevDead.append(dev)
		logp('I have discovered changes in serial devices:  new: '+ str(lDevNew)+ ' and dead: '+str(lDevDead), 'info')
	return [newListDev, lDevNew, lDevDead ]
	

#  main part of script

while True:
	# check if the list of serial has changed
	listDevNew=[]; listDevDead=[]
	[lastListDev, listDevNew, listDevDead] = giveUpdateListSerialDev(lastListDev, devSearchString)
	#
	# we remove dead dev from list of active dev
	for dev in listDevDead:
		if ( startedSerial2MQTT.has_key(dev) ):
			del startedSerial2MQTT[dev]
	#
	# if we detect new connected device, we try to connect python com script
	if (len (listDevNew) > 0):
		# we update list of scripts
		dSketch = readListSketchFromFile(fileNameListSketch)
		if (len(dSketch) == 0):
			logp('empty list of sketch configuration in file: ' + fileNameListSketch , 'error')
			exit(-1)
		# we try to connect python com script to new devices
		for ndl in listDevNew:
			logp('trying to recognize and connect new serial device: '+ ndl, 'info')
			idSketch = askIdSketchSerial(ndl)
			if dSketch.has_key(idSketch):
				launchSerial2MQTT(dSketch[idSketch], repTmp, ndl )
				# I am trusty, I immediately add it to active device list
				startedSerial2MQTT.update({ndl:idSketch})
			else:
				logp('I dont have this arduino id sketch in my listing file:' + idSketch, 'error')
	#
	# if there was an update, I log the new list of started dev
	if (len (listDevNew) > 0  or len (listDevDead) > 0):
		logp('list of started device:' + str(startedSerial2MQTT), 'info')
	#
	#
	time.sleep(sleepBetweenLoop)


