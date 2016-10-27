
#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

struct stListPin { int numPin; char *namePin; 
       stListPin(int n, char *np) : numPin(n), namePin(np) {}
};

#define PIN_CAPTOR  10
#define PIN_LED 2
stListPin listPin[] = {
  stListPin(PIN_LED, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknown")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);

String inputMessage = "";
boolean inputMessageReceived = false;
String msg2pyStart = "2py;";
String msg2mqttStart = "2mq;";
String msg2pyEnd = "\n";
String prefAT = "AT+";
String prefDO = "DO+";

struct Commande {
  String cmdName;
  int (*cmdFunction)(const String& topic_value) ;
  Commande(String cn, int (*cf)(const String&) ): cmdName(cn), cmdFunction(cf) {}
};

// list of available commandes (user) that the arduino will accept
int sendMessageStatus(const String& dumb);
int ledBlinkTime(const String& dumb);
int blinkTime=1000;
int switchLed(const String& dumb);

// list of available commandes (system ctrl) that the arduino will accept
int sendSketchId(const String& dumb);
int sendSketchBuild(const String& dumb);
int sendListCmdAT(const String& dumb);
int sendListCmdDO(const String& dumb);
int sendListPin(const String& dumb);
int cmdPinMode(const String& dumb);
int cmdPinRead(const String& dumb);
int cmdPinWrite(const String& dumb);

// list of available commands (user) that the arduino will accept
Commande cmdos[] = {
  Commande("SendValue", &sendMessageStatus),
  Commande("s",         &sendMessageStatus),
  Commande("ledBlink",  &ledBlinkTime),
  Commande("lit1/switch",    &switchLed)
};
int cmdosSize = sizeof(cmdos) / sizeof(Commande);

// list of available commands (system ctrl) that the arduino will accept
Commande cmds[] = {
  Commande("idSketch",  &sendSketchId),
  Commande("idBuild",   &sendSketchBuild),
  Commande("listCmdAT", &sendListCmdAT),
  Commande("listCmdDO", &sendListCmdDO),
  Commande("listPin",   &sendListPin),
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
  //blinkLed();
  
  // I slow down Arduino
  delay(10);
}



void serialEvent() 
{
  while(Serial.available()) 
  {
    char inChar = (char)Serial.read();
    
    if(inChar == '\n') 
      inputMessageReceived = true;
    else
      inputMessage += inChar;
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
          LOG_ERROR(String(F("DO cmd not recognized ! :")) + command);
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
              if (cr != 0)   {
                // I send pb msg
                String msg2py = msg2pyStart + cmds[i].cmdName + "/KO:" 
                               + cr + msg2pyEnd;
                Serial.print(msg2py);                
              }
              // if cr=0  I dont send OK msg, this is specific to function
          }
        }
        if (cmdNotFound)  {
          LOG_ERROR(String(F("AT cmd not recognized ! :")) + command);
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

int sendMessageStatus(const String& dumb)
{
    int sensorVal = getSensorValue(); 
    String msg2pyMosquitto = msg2mqttStart + "temp" + ":" + sensorVal 
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
int sendSketchId(const String& dumb)
{
    int sensorVal = getSensorValue(); 
    String sketchFullName = F(__FILE__);
    // __FILE__ contains the full path. we strip it
    int lastSlash = sketchFullName.lastIndexOf('/') +1;  // '/' is not for windows !
    String msg2py= msg2pyStart + "sketchId" + ":"  
                  + sketchFullName.substring(lastSlash) + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send an identifiant for the version of the sketch
//   we use the __TIME__ when it was built
int sendSketchBuild(const String& dumb)
{
    int sensorVal = getSensorValue(); 
    String msg2py = msg2pyStart + "sketchBuild" + ":" + __DATE__ 
                     + ", " + __TIME__ + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdAT(const String& dumb)
{
    String availCmd = "";
    if (cmdsSize >= 0)
       availCmd = cmds[0].cmdName;
    for (int i=1; i<cmdsSize; i++)
       availCmd += "," + cmds[i].cmdName;
    
    String msg2py = msg2pyStart + "listCmdAT" + ":" + availCmd 
                   + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdDO(const String& dumb)
{
    String availCmd = "";
    if (cmdosSize >= 0)
       availCmd = cmdos[0].cmdName;
    for (int i=1; i<cmdosSize; i++)
       availCmd += "," + cmdos[i].cmdName;
    
    String msg2py = msg2pyStart + "listCmdDO" + ":" + availCmd 
                   + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// send the list of Pin definition
int sendListPin(const String& dumb)
{
    String availPin = "";
    if (listPinSize >= 0)
       availPin = availPin + listPin[0].numPin +" "+ listPin[0].namePin;
    for (int i=1; i<listPinSize; i++)
       availPin = availPin + "," + listPin[i].numPin +" "+ listPin[i].namePin;
    
    String msg2py = msg2pyStart + "listPin" + ":" + availPin + msg2pyEnd;
    Serial.print(msg2py);
    return 0;
}

// change the blink period of the led
//   led will blink thanks to blinkLed called in loop
int ledBlinkTime(const String& sCmdAndBlinkTime)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sCmdAndBlinkTime.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("ledBlinkTime cmd needs 1 value"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sCmdAndBlinkTime.substring(ind+1);
    // value must be an int
    int iValue = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("ledBlinkTime: value must be 1 integer"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }
    else if (iValue < 0)   {
      LOG_ERROR(F("ledBlinkTime: value must be integer > 0"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 3;
    }

    blinkTime = iValue;
    
    // I send back OK msg
    String msg2py = msg2pyStart + "ledBlinkTime/OK:" + blinkTime + msg2pyEnd;
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

// switch the led On or Off
int switchLed(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("switchLed cmd needs 1 value"));
      String msg2py = msg2pyStart + "switchLed/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(F("switchLed: value must be ON or OFF"));
      String msg2py = msg2pyStart + F("switchLed/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 1;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      Serial.println ("jamais de la vie");
   
    digitalWrite(PIN_LED, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + F("switchLed/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

int cmdPinMode(const String& pin_mode) {
    // pin_mode contains cmd and value with this format cmd:value
    // value must exist
    int ind = pin_mode.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinMode cmd needs 2 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_mode.substring(ind+1);
    
    // we separate the 2 values
    ind = pin_mode.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinMode cmd needs 2 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinMode cmd: value 1 must be integer"));
      return 2;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinMode cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get 2nd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinMode cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 0) || (2 < iValue2))   {
      LOG_ERROR(F("pinMode cmd: value 2 must be mode=0/1/2"));
      return 5;
    }

    // ok, so we can make command
    pinMode(iValue1, iValue2);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinMode/OK" + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// cmd a analogRead or digitalRead
// value fmt: pin (0-20), 1=digitalRead/2=analogRead
int cmdPinRead(const String& pin_digitAnalog) {
    // pin_mode contains cmd and value with this format cmd:value
    
    // value must exist
    int ind = pin_digitAnalog.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinRead cmd needs 2 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_digitAnalog.substring(ind+1);
    
    // we separate the 2 values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinRead cmd needs 2 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinRead cmd: value 1 must be integer"));
      return 21;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinRead cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get 2nd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinRead cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 1) || (2 < iValue2))   {
      LOG_ERROR(F("pinRead cmd: value 2 must be digital/analog=1/2"));
      return 5;
    }

    // ok, so we can make command
    int iValue=0;
    if (iValue2 == 1)
      iValue = digitalRead(iValue1);
    else   // iValue2 == 2
      iValue = analogRead(iValue1);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinRead:" + iValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// cmd a digitalWrite or analogWrite (PWM) 
// value fmt: pin (0-20), 1=digitalRead/2=analogRead, value
int cmdPinWrite(const String& pin_digitAnalog_val) {
    // pin_digitAnalog_val contains cmd and value with this format cmd:pin,digAn,value
    
    // value must exist
    int ind = pin_digitAnalog_val.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_digitAnalog_val.substring(ind+1);
    
    // we separate the values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 1 must be integer"));
      return 21;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinWrite cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get the 2 last values
    sValues = sValues.substring(ind+1);
    // we separate the 2 last values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 31;
    }

    // we get 2nd value
    sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 1) || (2 < iValue2))   {
      LOG_ERROR(F("pinWrite cmd: value 2 must be digital/analog=1/2"));
      return 5;
    }

    // we get 3rd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue3 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue3 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 3 must be integer"));
      return 6;
    }
    else if (iValue3 < 0)   {
      LOG_ERROR(F("pinWrite cmd: value 3 must be >=0"));
      return 7;
    }

    // ok, so we can make command
    if (iValue2 == 1)
      digitalWrite(iValue1,iValue3);
    else   // iValue2 == 2
      analogWrite(iValue1,iValue3);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinWrite/OK" + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
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
      Serial.println(String(F("msg incomplete:")) + inputMessage + F(":end"));
      lastPrint = millis();
      return 1;
    }
  }
  
  return 0;
}

