
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
String prefDO = "DO+";

struct Commande {
  String cmdName;
  int (*cmdFunction)(String topic_value) ;
  Commande(String cn, int (*cf)(String) ): cmdName(cn), cmdFunction(cf) {}
};

// list of available commandes that the arduino will accept
int sendMessageStatus(String dumb);
int ledBlinkTime(String dumb);
int blinkTime=1000;

// list of available commandes that the arduino will accept
int sendSketchId(String dumb);
int sendSketchBuild(String dumb);
int sendListCmdDO(String dumb);
int sendListCmdAT(String dumb);
int cmdPinMode(String dumb);
int cmdPinRead(String dumb);
int cmdPinWrite(String dumb);

// list of available commands (user) that the arduino will accept
Commande cmdos[] = {
  Commande("SendValue", &sendMessageStatus),
  Commande("s",         &sendMessageStatus),
  Commande("led",       &ledBlinkTime)
};
int cmdosSize = sizeof(cmdos) / sizeof(Commande);

// list of available commands (system ctrl) that the arduino will accept
Commande cmds[] = {
  Commande("idSketch",  &sendSketchId),
  Commande("idBuild",   &sendSketchBuild),
  Commande("listCmdDO", &sendListCmdDO),
  Commande("pinMode",   &cmdPinMode),
  Commande("pinRead",   &cmdPinRead),
  Commande("pinWrite",  &cmdPinWrite)
};
int cmdsSize = sizeof(cmds) / sizeof(Commande);


void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(9600);
    inputMessage.reserve(200);
    
    // I send identification of sketch
    sendSketchId("");
    sendSketchBuild("");
}
 
void loop()
{
  checkMessageReceived();

  // because of bad communication, some message may be stucked in
  //   serial buffer. If so, we trace it
  checkNoStuckMessageInBuffer();

  // blink Led. blink time is set with external cmd ledBlinkTime
  blinkLed();
  
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
      // if it is a cmd for user
      if (inputMessage.startsWith(prefDO))
      {
        LOG_DEBUG(F("Message is a DO cmd, I am parsing..."));
        
        String command = inputMessage.substring(prefDO.length());
        bool cmdoNotFound = true;
        for (int i=0; i<cmdosSize; i++)
        {
          if (command.startsWith(cmdos[i].cmdName))
          {
              cmdoNotFound = false;
              int cr = cmdos[i].cmdFunction(command);
          }
        }
        if (cmdoNotFound)  {
          LOG_ERROR(String("DO cmd not recognized ! :") + command);
        }
      }
      // if it is a cmd for system ctrl
      else if (inputMessage.startsWith(prefAT))
      {
        LOG_DEBUG(F("Message is a AT cmd, I am parsing..."));
        
        String command = inputMessage.substring(prefAT.length());
        bool cmdNotFound = true;
        for (int i=0; i<cmdsSize; i++)
        {
          if (command.startsWith(cmds[i].cmdName))
          {
              cmdNotFound = false;
              int cr = cmds[i].cmdFunction(command);
          }
        }
        if (cmdNotFound)  {
          LOG_ERROR(String("AT cmd not recognized ! :") + command);
        }
      }
      else   {
        LOG_DEBUG(F("Message is not a AT or DO cmd"));
      }
      inputMessage = "";
      inputMessageReceived = false;
  }
}


/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

int sendMessageStatus(String dumb)
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

/*---------------------------------------------------------------*/
/*                  list of system ctrl function                 */
/*---------------------------------------------------------------*/

// send an identiant of arduino sketch: it sends the name of the file !
int sendSketchId(String dumb)
{
    int sensorVal = getSensorValue(); 
    String sketchFullName = F(__FILE__);
    // __FILE__ contains the full path. we strip it
    int lastSlash = sketchFullName.lastIndexOf('/') +1;  // '/' is not for windows !
    String msg2py= msg2pyStart + "/sketchId" + ":"  
                  + sketchFullName.substring(lastSlash) + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send an identifiant for the version of the sketch
//   we use the __TIME__ when it was built
int sendSketchBuild(String dumb)
{
    int sensorVal = getSensorValue(); 
    String msg2py = msg2pyStart + "/sketchBuild" + ":" + __DATE__ 
                     + ", " + __TIME__ + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdAT(String dumb)
{
    String availCmd = "";
    if (cmdsSize >= 0)
       availCmd = cmds[0].cmdName;
    for (int i=1; i<cmdsSize; i++)
       availCmd += "," + cmds[i].cmdName;
    
    String msg2py = msg2pyStart + "/listCmdAT" + ":" + availCmd 
                   + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdDO(String dumb)
{
    String availCmd = "";
    if (cmdosSize >= 0)
       availCmd = cmdos[0].cmdName;
    for (int i=1; i<cmdosSize; i++)
       availCmd += "," + cmdos[i].cmdName;
    
    String msg2py = msg2pyStart + "/listCmdDO" + ":" + availCmd 
                   + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// 
int ledBlinkTime(String sCmdAndBlinkTime)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sCmdAndBlinkTime.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR("ledBlinkTime cmd needs 1 value");
      String msg2py = msg2pyStart + "/ledBlinkTimeKO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sCmdAndBlinkTime.substring(ind+1);
    // value must be an int
    int iValue = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR("ledBlinkTime: value must be 1 integer");
      String msg2py = msg2pyStart + "/ledBlinkTimeKO" + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }
    else if (iValue < 0)   {
      LOG_ERROR("ledBlinkTime: value must be integer > 0");
      String msg2py = msg2pyStart + "/ledBlinkTimeKO" + msg2pyEnd;
      Serial.print(msg2py);
      return 3;
    }


    blinkTime = iValue;
    
    // I send back OK msg
    String msg2py = msg2pyStart + "/ledBlinkTimeOK" + ":" + blinkTime 
                   + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// tha function is called in the loop
// it blinks LED at speed blinkTime (global variable)
void blinkLed() {
  static long lastChange=0;
  static int  blinkState=0;

  if (millis() - lastChange > blinkTime)   {
    lastChange = millis();
    blinkState = ! blinkState;
    digitalWrite(PIN_LED, blinkState);
  }
}

int cmdPinMode(String dumb) {
  ;
}
int cmdPinRead(String dumb) {
  ;
}
int cmdPinWrite(String dumb) {
  ;
}

// because of bad communication, some messages may be stucked in
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

