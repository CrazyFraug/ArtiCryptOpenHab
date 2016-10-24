
#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#define PIN_CAPTOR
#define PIN_LED 13
 
#define BLINK_DELAY 1000
 
String inputMessage = "";
boolean inputMessageReceived = false;
String msg2pyStart = "2py;";
String msg2mqttStart = "2mq;";
String msg2pyEnd = "\n";
String prefAT = "AT+";

struct Commande {
  String cmdName;
  int (*cmdFunction)() ;
  Commande(String cn, int (*cf)() ): cmdName(cn), cmdFunction(cf) {}
};

// list of available commandes that the arduino will accept
int sendMessageStatus();

Commande cmds[] = {
  Commande("SendValue", &sendMessageStatus),
  Commande("s", &sendMessageStatus),
  Commande("o", &sendMessageStatus)
};
int cmdsSize = sizeof(cmds) / sizeof(Commande);

void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(9600);
    inputMessage.reserve(200);
    
    // I send identification of sketch
    sendSketchId();
    sendSketchBuild();
}
 
void loop()
{
  checkMessageReceived();

  // because of bad communication, some message may be stucked in
  //   serial buffer. If so, we trace it
  checkNoStuckMessageInBuffer();
  
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
      LOG_DEBUG(F("MessageReceived, I am parsing..."));
      
      // if it is a cmd
      if (inputMessage.startsWith(prefAT))
      {
        LOG_DEBUG(F("Message is a AT cmd, I am parsing..."));
        
        String command = inputMessage.substring(prefAT.length());
        bool cmdNotFound = true;
        for (int i=0; i<cmdsSize; i++)
        {
          if (command.startsWith(cmds[i].cmdName))
          {
              cmdNotFound = false;
              int cr = cmds[i].cmdFunction();
          }
        }
        if (cmdNotFound)  {
          LOG_ERROR(String("AT cmd not recognized ! :") + command);
        }
      }
      else   {
        LOG_DEBUG(F("Message is not a AT cmd"));
      }
      inputMessage = "";
      inputMessageReceived = false;
  }
}

int sendMessageStatus()
{
    int sensorVal = getSensorValue(); 
    String msg2pyMosquitto = msg2mqttStart + "temp/0" + ":" + sensorVal 
                            + msg2pyEnd;
    Serial.print(msg2pyMosquitto);
    return 0;
}

int getSensorValue()
{
  // fake sensor value
  return millis() % 1024; 
}

void sendSketchId()
{
    int sensorVal = getSensorValue(); 
    String sketchFullName = F(__FILE__);
    int lastSlash = sketchFullName.lastIndexOf('/') +1;
    String msg2pyMosquitto = msg2pyStart + "/sketchId" + ":"  
                            + sketchFullName.substring(lastSlash) + msg2pyEnd;
    Serial.print(msg2pyMosquitto);
}

void sendSketchBuild()
{
    int sensorVal = getSensorValue(); 
    String msg2pyMosquitto = msg2pyStart + "/sketchBuild" + ":" + __DATE__ 
                            + ", " + __TIME__ + msg2pyEnd;
    Serial.print(msg2pyMosquitto);
}

// because of bad communication, some message may be stucked in
//   serial buffer. If so, we trace it
int  checkNoStuckMessageInBuffer() {
  static bool msgHasBegun;
  static long lastPrint = millis();

  // we update if a msg has just begun 
  if (inputMessage.length()>0)   {
    if ( ! msgHasBegun )   {
      msgHasBegun = true;
      lastPrint = millis();
    }
  }
  else
     msgHasBegun = false;
  
  // I display input message every second, if it is not complete
  // It should not happen since the message will be processed too quickly
  if (( ! inputMessageReceived ) && (msgHasBegun)) {
    if ( (millis() - lastPrint > 1000) && 
         inputMessage.length()>0 ) {
      Serial.println(String("msg incomplete:") + inputMessage + F(":end"));
      lastPrint = millis();
      return 1;
    }
  }
  
  return 0;
}

