#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#include "msgFromMQTT.h"
String sketchFullName = __FILE__;


#define PIN_LED_A     3
#define PIN_LED_B     5
#define PIN_LED_HEAT  6
#define PIN_LED_DA    9
stListPin listPin[] = {
  stListPin(PIN_LED_A,    "lumiere1"),
  stListPin(PIN_LED_B,    "lumiere2"),
  stListPin(PIN_LED_HEAT, "LED heater"),
  stListPin(PIN_LED_DA,   "lumiereDimmer")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);


// list of available commands (user) that the arduino will accept
int switchLed1(const String& dumb);
int switchLed2(const String& dumb);
int ledHeater(const String& dumb);
int dimmerLed1(const String& dumb);

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept
Commande cmdos[] = {
  Commande("radiateur/switch",  &ledHeater),
  Commande("cuisine/switch",    &switchLed1),
  Commande("repas/switch",      &switchLed2),
  Commande("lumiere/dimmer",    &dimmerLed1)
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
    pinMode(PIN_LED_A, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    pinMode(PIN_LED_HEAT, OUTPUT);
    
    Serial.begin(9600);
    inputMessage.reserve(200);
    
    // I send identification of sketch
    sendSketchId("");
    sendSketchBuild("");
    sendListPin("");
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
      String msg2py = msg2pyStart + sOnOff + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = sOnOff.substring(0, ind);
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(command+ F(": value must be ON or OFF"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 50;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      Serial.println ("jamais de la vie");
   
    analogWrite(PIN_LED_A, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("cuisine/etat:") + sValue + msg2pyEnd;
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
      String msg2py = msg2pyStart + sOnOff + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = sOnOff.substring(0, ind);
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(command+ F(": value must be ON or OFF"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 50;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      Serial.println ("jamais de la vie");
   
    digitalWrite(PIN_LED_B, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("repas/etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// switch the led On or Off
int ledHeater(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("cmd needs 1 value"));
      String msg2py = msg2pyStart + sOnOff + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = sOnOff.substring(0, ind);
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(command+ F(":value must be ON or OFF"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
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
   
    digitalWrite(PIN_LED_HEAT, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("radiateur/etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}


// dimmer the led from 0 to 100 --> PWM  from 0 to 255
int dimmerLed1(const String& sDimmer100)
{
    // input arg  contains cmd and value with this format cmd:value
    // value must exist
    int ind = sDimmer100.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("dimmerLed cmd needs 1 value"));
      String msg2py = msg2pyStart + sDimmer100 + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = sDimmer100.substring(0, ind);
    // we get value part
    String sValue = sDimmer100.substring(ind+1);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(command+ F(": value 1 must be integer"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }
    else if ((iValue1 < 0) || (100 < iValue1))   {
      LOG_ERROR(command+ F(": value 1 must be between 0 and 100"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 3;
    }

    // converts iValue1 0/100  to  iValue PWM 0/255
    int iValue = int(iValue1 * 2.55);
    
    analogWrite(PIN_LED_DA, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("lumiere/etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

