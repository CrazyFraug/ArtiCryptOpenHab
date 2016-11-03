#!/usr/bin/python
 
import paho.mqtt.client as mqtt
import serial
import time
 
ser = serial.Serial('/dev/ttyUSB1', 9600)
mqttc = mqtt.Client()
mqttc.connect('localhost', port=1883, keepalive=60, bind_address="")
 
while True:
    time.sleep(1)
    ser.write("s\n");
    response = ser.readline().replace('\n', '')
    tags = response.split(';')
    for tag in tags:
        tag = tag.strip()
        if tag and len(tag.split(':')) == 2:
            (topic, value) = tag.split(':')
            print('{} = {}'.format(topic, value))
            mqttc.publish('arduino/0/' + topic, value)

