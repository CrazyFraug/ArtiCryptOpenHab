
#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#define PIN_CAPTOR
#define PIN_LED 13
 
#define BLINK_DELAY 1000
 
String inputMessage = "";
boolean inputMessageReceived = false;
String msg2pyStart = "m2p;";
String msg2pyEnd = "\n";
String prefAT = "AT+";

String cmdSendValue="SendValue";
String cmdSendValueShort="s";

void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(57600);
    inputMessage.reserve(200);
}
 
void loop()
{
  checkMessageReceived();

  // I slow down Arduino
  delay(10);
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
 
void checkMessageReceived()
{
    if(inputMessageReceived) 
    {   
        LOG_DEBUG("MessageReceived, I am parsing...");
        if (inputMessage.startsWith(prefAT))
        {
          LOG_DEBUG("Message is a AT cmd, I am parsing...");
          String command = inputMessage.substring(prefAT.length());
          if (command.startsWith(cmdSendValue))
          {
              sendMessageStatus();
          }
          else if (command.startsWith(cmdSendValueShort))
          {
              sendMessageStatus();
          }
          else  {
            LOG_ERROR(String("AT cmd not recognized ! :") + command);
          }          
        }
        inputMessage = "";
        inputMessageReceived = false;
    }
  
}
void sendMessageStatus()
{
    int sensorVal = getSensorValue(); 
    String msg2pyMosquitto = msg2pyStart + "artilect/0/temp/0" + ":" + sensorVal 
                            + msg2pyEnd;
    Serial.print(msg2pyMosquitto);
    // trace
    Serial.print("sending msg; value is:");
    Serial.println(sensorVal);
}

int getSensorValue()
{
  // fake sensor value
  return millis() % 1024; 
}


