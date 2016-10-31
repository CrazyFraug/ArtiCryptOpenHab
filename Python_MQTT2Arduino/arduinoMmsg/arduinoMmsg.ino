#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#include "msgFromMQTT.h"
String sketchFullName = __FILE__;


#define PIN_CAPTOR  10
#define PIN_LED 13
stListPin listPin[] = {
  stListPin(PIN_LED, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknown")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);


// list of available commandes (user) that the arduino will accept
int switchLed1(const String& dumb);

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept
Commande cmdos[] = {
  Commande("SendValue",     &sendMessageStatus),
  Commande("S",             &sendMessageStatus),
  Commande("ledBlink",      &ledBlinkTime),
  Commande("lit1/switch",   &switchLed1)
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


/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

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
  serialEventMFMQTT();
}

/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

// switch the led On or Off
int switchLed1(const String& sOnOff)
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

// switch the led On or Off
int switchLed2(const String& sOnOff)
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


