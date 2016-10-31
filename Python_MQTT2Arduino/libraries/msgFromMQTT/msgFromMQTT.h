#ifndef MSGFROMMQTT_H
#define MSGFROMMQTT_H

#if defined(CALL_FROM_MSG_FROM_MQTT)
#define EXTERN_MFMQTT 
extern String sketchFullName ;
#else
#define EXTERN_MFMQTT extern
#endif

// You have to define sketchFullName in your main file sketch.ino, this way
//String sketchFullName = __FILE__;


struct stListPin { int numPin; char *namePin; 
       stListPin(int n, char *np) : numPin(n), namePin(np) {}
};

// listPin and listPinSize are defined in main sketch.ino
// ex of definition
/*
// #define PIN_CAPTOR  10
// #define PIN_LED 2
stListPin listPin[] = {
  stListPin(PIN_LED, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknown")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);
*/
#define PIN_LED13 13

struct Commande {
  String cmdName;
  int (*cmdFunction)(const String& topic_value) ;
  Commande(String cn, int (*cf)(const String&) ): cmdName(cn), cmdFunction(cf) {}
};

#if defined(CALL_FROM_MSG_FROM_MQTT)
String inputMessage = "";
boolean inputMessageReceived = false;
String msg2pyStart = "2py;";
String msg2mqttStart = "2mq;";
String msg2pyEnd = "\n";
String prefAT = "AT+";
String prefDO = "DO+";
#else
extern boolean inputMessageReceived;
extern String inputMessage, msg2pyStart, msg2mqttStart, msg2pyEnd, prefAT, prefDO ;
#endif
// You can keep all those functions here
//   they DO NOT take program space if you dont call them in main sketch.ino
//   that means if they are not included in your cmdos and cmds  arrays

// list of available commands (user) that the arduino will accept
int sendMessageStatus(const String& dumb);
int ledBlinkTime(const String& dumb);
// uses  int blinkTime=1000;    defined in .cpp
int switchLed(const String& dumb);

// list of available commands (system ctrl) that the arduino will accept
int sendSketchId(const String& dumb);
int sendSketchBuild(const String& dumb);
int sendListCmdAT(const String& dumb);
int sendListCmdDO(const String& dumb);
int sendListPin(const String& dumb);
int cmdPinMode(const String& dumb);
int cmdPinRead(const String& dumb);
int cmdPinWrite(const String& dumb);

// list of available commands (user) that the arduino will accept
// ex  that you define in main sketch.ino
/*
Commande cmdos[] = {
  Commande("SendValue", &sendMessageStatus),
  Commande("s",         &sendMessageStatus),
  Commande("ledBlink",  &ledBlinkTime),
  Commande("lit1/switch",    &switchLed)
};
int cmdosSize = sizeof(cmdos) / sizeof(Commande);

// list of available commands (system ctrl) that the arduino will accept
// ex  that you define in main sketch.ino
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
*/

// you need to read serial and empty buffer into inputMessage
// for this you can call  serialEventMFMQTT in serialEvent function, like that
/*
void serialEvent() 
{
  serialEventMFMQTT();
}
*/

void serialEventMFMQTT();

// because of bad communication, some messages may be stucked in
//   serial buffer. If so, we trace it
int  checkNoStuckMessageInBuffer() ;

// check if a message has been received and analyze it
// in your main sketch.ino, you have to update global var inputMessageReceived and inputMessage
// checkMessageReceived calls the function corresponding to  cmds/cmdos array
void checkMessageReceived();

int getSensorValue(); 


#endif // MSGFROMMQTT_H
