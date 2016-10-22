#include <OneWire.h>
#include <DallasTemperature.h>
 
#define PIN_DALLAS_ONE_WIRE_BUS 3
#define PIN_LED 13
#define SEPARATOR ";"
 
#define BLINK_DELAY 1000
 
OneWire dallasOneWireBus(PIN_DALLAS_ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&dallasOneWireBus);
String inputMessage = "";
boolean inputMessageReceived = false;
 
void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(9600);
    temperatureSensors.begin();
    inputMessage.reserve(200);
}
 
void loop()
{
    if(inputMessageReceived) 
    {   
        char command = inputMessage.charAt(0);
        if(command == 's')
        {
            sendMessageStatus();
        }
        inputMessage = "";
        inputMessageReceived = false;
    }
}
 
void serialEvent() 
{
  while(Serial.available()) 
  {
    char inChar = (char)Serial.read();
    inputMessage += inChar;
    if(inChar == '\n') 
    {
      inputMessageReceived = true;
    }
  }
}
 
void sendMessageStatus()
{
    temperatureSensors.requestTemperatures(); 
     
    for(int i = 0; i < temperatureSensors.getDeviceCount(); i ++)
    {
        Serial.print("temp/");
        Serial.print(i);
        Serial.print(":");
        Serial.print(temperatureSensors.getTempCByIndex(i));
        Serial.print(SEPARATOR); 
    }
    Serial.println("");
}

