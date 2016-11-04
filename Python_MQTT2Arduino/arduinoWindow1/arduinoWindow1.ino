#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#include "msgFromMQTT.h"
String sketchFullName = __FILE__;


#define PIN_MOT_A     3
#define PIN_MOT_B     5
#define PIN_MOT_C     6
#define PIN_MOT_D     9
stListPin listPin[] = {
  stListPin(PIN_MOT_A,    "moteur1"),
  stListPin(PIN_MOT_B,    "moteur2"),
  stListPin(PIN_MOT_C,    "moteur3"),
  stListPin(PIN_MOT_D,    "moteur4")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);


// list of available commands (user) that the arduino will accept
int switchMoteur1(const String& dumb);
int switchMoteur2(const String& dumb);
int switchMoteur3(const String& dumb);
int switchMoteur4(const String& dumb);
int settingMoteurs(const String& dumb);

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept
Commande cmdos[] = {
  Commande("moteur1/switch",  &switchMoteur1),
  Commande("moteur2/switch",  &switchMoteur2),
  Commande("moteur3/switch",  &switchMoteur3),
  Commande("moteur4/switch",  &switchMoteur4),
  Commande("moteur/setting",  &settingMoteurs)
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


int switchMoteurAny(const String& sOnOff, const int numMoteur);

struct Moteur  {
  int stateUpDown;       // 1 /  -1
  int actionUpDownNone;  // 1 / 0 / -1
  long timeEndAction;
  int valStill;
  int valUp;
  int valDown;
  int periodUp;
  int periodDown;
  int pin;
  void up();
  void down();
  Moteur (int vs, int peru, int perd): stateUpDown(-1), actionUpDownNone(0),
      timeEndAction(0), valStill(vs), valUp(vs+20), valDown(vs-20),
      periodUp(peru), periodDown(perd)   {}
};

void moteurCheck();

Moteur moteurs[] = {
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000),
  Moteur(100, 1000, 1000)
};
const int NBMOT = sizeof(moteurs) / sizeof(Moteur);

/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

void setup()
{
    for (int i=0; i<NBMOT; i++)
        pinMode(moteurs[i].pin, OUTPUT);
    
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

  // check and update the movement of moteurs
  moteurCheck();
  
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

void moteurCheck() 
{
  long now = millis();
  for (int im=0; im<NBMOT; im++)
  {
    // update action
    if (now > moteurs[im].timeEndAction)   {
      // I update position state
      if (moteurs[im].actionUpDownNone == -1)   // down
          moteurs[im].stateUpDown = -1;
      else if (moteurs[im].actionUpDownNone == 1)   // up
          moteurs[im].stateUpDown = 1;
      
      moteurs[im].actionUpDownNone = 0;   // no action
    }

    // I send value corresponding to action
    if (moteurs[im].actionUpDownNone == -1)   // DOWN
      analogWrite(moteurs[im].pin, moteurs[im].valDown);
    else if (moteurs[im].actionUpDownNone == 1)   // UP
      analogWrite(moteurs[im].pin, moteurs[im].valUp);
    else   //  actionUpDownNone == nothing
      analogWrite(moteurs[im].pin, moteurs[im].valStill);
  }
}

// switch the roller shutter ( or  window ) up or down
// this function is common to anyone
int switchMoteurAny(const String& sOnOff, const int numMoteur)
{
    // sOnOff contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("switchMoteur cmd needs 1 value"));
      String msg2py = msg2pyStart + sOnOff + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = sOnOff.substring(0, ind);
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  UP  or  DOWN
    if ( ( ! sValue.equals("UP")) && ( ! sValue.equals("DOWN")) )   {
      LOG_ERROR(command+ F(": value must be UP or DOWN"));
      String msg2py = msg2pyStart + command + F("/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    // commands motors
    if (sValue.equals("UP"))
      moteurs[numMoteur].up();
    else if (sValue.equals("DOWN"))
      moteurs[numMoteur].down();
    else
      Serial.println ("jamais de la vie");
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("moteur")+numMoteur+F("/etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// switch the moteur up or down
int switchMoteur1(const String& sOnOff)
{
    return switchMoteurAny(sOnOff, 1);
}

int switchMoteur2(const String& sOnOff) { return switchMoteurAny(sOnOff, 2); }
int switchMoteur3(const String& sOnOff) { return switchMoteurAny(sOnOff, 3); }
int switchMoteur4(const String& sOnOff) { return switchMoteurAny(sOnOff, 4); }
int switchMoteur5(const String& sOnOff) { return switchMoteurAny(sOnOff, 5); }
int switchMoteur6(const String& sOnOff) { return switchMoteurAny(sOnOff, 6); }
int switchMoteur7(const String& sOnOff) { return switchMoteurAny(sOnOff, 7); }
int switchMoteur8(const String& sOnOff) { return switchMoteurAny(sOnOff, 8); }


// set value for moteurs
int settingMoteurs(const String& aCmdVal)
{
    // input arg  contains cmd and value with this format cmd:value
    // value must exist
    int ind = aCmdVal.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("moteurSetting cmd needs 4 values: mot:still:perup:perdown"));
      String msg2py = msg2pyStart + aCmdVal + "/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get cmd part
    String command = aCmdVal.substring(0, ind);
    // we get value part
    String sValue = aCmdVal.substring(ind+1);
    for (int i=0; i<4; i++)   {
      // value must be an int
      int iValue1 = sValue.toInt();
      // toInt will return 0, if it is not an int
      if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
        LOG_ERROR(command+ F(": value mot 1 must be integer"));
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
    }

    // converts iValue1 0/100  to  iValue PWM 0/255
    int iValue = int(iValue1 * 2.55);
    
    analogWrite(PIN_MOT_A, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + command + F("/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("lumiere/etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// trigger action upward
void Moteur::up()   {
  // we return immediately if it was already UP (for fear of echo)
  if (actionUpDownNone == 1)
      return;
  
  actionUpDownNone = 1;
  // the first time, it sets time of end
  timeEndAction = millis() + periodUp;
}
// trigger action downward
void Moteur::down()   {
  // we return immediately if it was already UP (for fear of echo)
  if (actionUpDownNone == -1)
      return;
  
  actionUpDownNone = -1;
  // the first time, it sets time of end
  timeEndAction = millis() + periodDown;
}

