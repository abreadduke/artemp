#include <string.h>

#define A 9
#define B 2
#define C 5
#define D 8
#define E 7
#define F 3
#define G 4
#define DP 6
#define D1 13
#define D2 12
#define D3 11
#define D4 10
#define BUTTON_PIN 14

enum DeviceMode{
  INITIALISATION,
  DATE,
  ATMP
};
extern enum DeviceMode devmode;

void setup() {
  Serial.begin(115200);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DP, OUTPUT);

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);

  pinMode(BUTTON_PIN, INPUT);
}
void setSegmentActive(byte segment, boolean active){
  switch(segment){
    case 0:{
      digitalWrite(D4, !active);
      break;
    }
    case 1:{
      digitalWrite(D3, !active);
      break;
    }
    case 2:{
      digitalWrite(D2, !active);
      break;
    }
    case 3:{
      digitalWrite(D1, !active);
      break;
    }
  }
}
void setSegment(byte number){
  switch(number){
    case 0:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, HIGH);
      digitalWrite(F, HIGH);
      digitalWrite(G, LOW);
      break;
    }
    case 1:{
      digitalWrite(A, LOW);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, LOW);
      digitalWrite(E, LOW);
      digitalWrite(F, LOW);
      digitalWrite(G, LOW);
      break;
    }
    case 2:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, LOW);
      digitalWrite(D, HIGH);
      digitalWrite(E, HIGH);
      digitalWrite(F, LOW);
      digitalWrite(G, HIGH);
      break;
    }
    case 3:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, LOW);
      digitalWrite(F, LOW);
      digitalWrite(G, HIGH);
      break;
    }
    case 4:{
      digitalWrite(A, LOW);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, LOW);
      digitalWrite(E, LOW);
      digitalWrite(F, HIGH);
      digitalWrite(G, HIGH);
      break;
    }
    case 5:{
      digitalWrite(A, HIGH);
      digitalWrite(B, LOW);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, LOW);
      digitalWrite(F, HIGH);
      digitalWrite(G, HIGH);
      break;
    }
    case 6:{
      digitalWrite(A, HIGH);
      digitalWrite(B, LOW);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, HIGH);
      digitalWrite(F, HIGH);
      digitalWrite(G, HIGH);
      break;
    }
    case 7:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, LOW);
      digitalWrite(E, LOW);
      digitalWrite(F, LOW);
      digitalWrite(G, LOW);
      break;
    }
    case 8:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, HIGH);
      digitalWrite(F, HIGH);
      digitalWrite(G, HIGH);
      break;
    }
    case 9:{
      digitalWrite(A, HIGH);
      digitalWrite(B, HIGH);
      digitalWrite(C, HIGH);
      digitalWrite(D, HIGH);
      digitalWrite(E, LOW);
      digitalWrite(F, HIGH);
      digitalWrite(G, HIGH);
      break;
    }
    case -1:{
      digitalWrite(A, LOW);
      digitalWrite(B, LOW);
      digitalWrite(C, LOW);
      digitalWrite(D, LOW);
      digitalWrite(E, LOW);
      digitalWrite(F, LOW);
      digitalWrite(G, LOW);
      break;
    }
  }
}
struct dissegs{
  byte qs;
  byte ts;
  byte ds;
  byte ss;
  byte dm;
};
byte changeModeOnDisplay(){
  switch(devmode){
    case DATE:{
      //0001
      return 1;
    }
    case ATMP:{
      //0010
      return 2;
    }
    default:{
      //1111
      return 15;
    }
  }
}
struct dissegs setDisplay(short number){
  byte qs = static_cast<byte>(number / 1000);
  byte ts = static_cast<byte>((number / 100) % 10);
  byte ds = static_cast<byte>((number / 10) % 10);
  byte ss = static_cast<byte>(number % 10);
  byte dm = changeModeOnDisplay();
  return {qs, ts, ds, ss, dm};
}
void draw(struct dissegs* dis){
  setSegmentActive(0, false);
  setSegmentActive(1, false);
  setSegmentActive(2, false);
  setSegmentActive(3, false);
  digitalWrite(DP, LOW);

  setSegmentActive(0, true);
  setSegment(dis->ss);
  if((dis->dm & 1) == 1) digitalWrite(DP, HIGH);
  delay(1);

  setSegmentActive(0, false);
  setSegmentActive(1, false);
  setSegmentActive(2, false);
  setSegmentActive(3, false);
  digitalWrite(DP, LOW);

  setSegmentActive(1, true);
  setSegment(dis->ds);
  if(((dis->dm >> 1) & 1) == 1) digitalWrite(DP, HIGH);
  delay(1);

  setSegmentActive(0, false);
  setSegmentActive(1, false);
  setSegmentActive(2, false);
  setSegmentActive(3, false);
  digitalWrite(DP, LOW);

  setSegmentActive(2, true);
  setSegment(dis->ts);
  if(((dis->dm >> 2) & 1) == 1) digitalWrite(DP, HIGH);
  delay(1);

  setSegmentActive(0, false);
  setSegmentActive(1, false);
  setSegmentActive(2, false);
  setSegmentActive(3, false);
  digitalWrite(DP, LOW);

  setSegmentActive(3, true);
  setSegment(dis->qs);
  if(((dis->dm >> 3) & 1) == 1) digitalWrite(DP, HIGH);
  delay(1);
}
int number = 0;
unsigned long last_time = 0;
class StateMachine{
  public:
  int state = 0;
  virtual void next(){
    this->state++;
  }
};
#define MSGC_IOCONTROL 0
#define MSGC_READDATNUMBERSTATE 1
#define MSGC_PREPARE_SETUP_DATE_MODE_ON_HOST 2
#define MSGC_SETUP_DATE_MODE_ON_HOST 3
#define MSGC_PREPARE_SETUP_ATMP_MODE_ON_HOST 4
#define MSGC_SETUP_ATMP_MODE_ON_HOST 6

DeviceMode devmode = DATE;
DeviceMode newdevmode = DATE;
DeviceMode hostmode = INITIALISATION;
class MessageController : protected StateMachine{
  protected:
  bool cycleSynced = true;
  bool connectionStatus = false;
  private:
  char msg[5];
  void __msgcCallReadDat(){
    if(Serial.available() >= 4){
      Serial.readBytes((byte*)&number, 4);
      state=MSGC_IOCONTROL;
    }
  }
  void __setupDateModeOnHost(){
    if(Serial.availableForWrite() >= 5){
      Serial.write("date!", 5);
      state=MSGC_IOCONTROL;
    }
  }
  void __prepareSetupDateModeOnHost(){
    if(Serial.available() >= 4){
      Serial.readBytes(msg, 4);
      if(strcmp(msg, "read\0")==0) {
        hostmode = DATE;
        state=MSGC_SETUP_DATE_MODE_ON_HOST;
      }
    }
  }
  void __prepareSetupATMPModeOnHost(){
    if(Serial.available() >= 4){
      Serial.readBytes(msg, 4);
      if(strcmp(msg, "read\0")==0) {
        hostmode = ATMP;
        state=MSGC_SETUP_ATMP_MODE_ON_HOST;
      }
    }
  }
  void __setupATMPModeOnHost(){
    if(Serial.availableForWrite() >= 5){
      Serial.write("atmp!", 5);
      state=MSGC_IOCONTROL;
    }
  }
  bool readyToWrite(char* writeMsg, int length){
    if(Serial.availableForWrite() >= length){
      Serial.write(writeMsg, length);
      return true;
    } else return false;
  }
  void __msgcIOController(){
    if(Serial.available() >= 4 && !connectionStatus){
      Serial.readBytes(msg, 4);
      msg[4]='\0';
      if(strcmp(msg, "init\0") == 0 && Serial.availableForWrite() >= 5){
        Serial.write("conn!", 5);
        connectionStatus = true;
      }
    }
    devmode = newdevmode;
    //DATE mode control
    if(Serial.availableForWrite() >= 5 && connectionStatus == true && devmode == DATE){
      if(hostmode != DATE){
        Serial.write("chmd!", 5);
        state=MSGC_PREPARE_SETUP_DATE_MODE_ON_HOST;
        return;
      }
      else if(hostmode == DATE){
        Serial.write("keep!", 5);
        state=MSGC_READDATNUMBERSTATE;
        return;
      }
    }
    //ATMP mode control
    if(Serial.availableForWrite() >= 5 && connectionStatus == true && devmode == ATMP){
      if(hostmode != ATMP){
        Serial.write("chmd!", 5);
        state=MSGC_PREPARE_SETUP_ATMP_MODE_ON_HOST;
        return;
      }
      else if(hostmode == ATMP){
        Serial.write("keep!", 5);
        state=MSGC_READDATNUMBERSTATE;
        return;
      }
    }
  }
  public:
  virtual void next() override{
    switch(state){
      case MSGC_READDATNUMBERSTATE:{
        __msgcCallReadDat();
        break;
      }
      case MSGC_IOCONTROL:{
        __msgcIOController();
        break;
      }
      case MSGC_SETUP_DATE_MODE_ON_HOST:{
        __setupDateModeOnHost();
        break;
      }
      case MSGC_PREPARE_SETUP_DATE_MODE_ON_HOST:{
        __prepareSetupDateModeOnHost();
        break;
      }
      case MSGC_PREPARE_SETUP_ATMP_MODE_ON_HOST:{
        __prepareSetupATMPModeOnHost();
        break;
      }
      case MSGC_SETUP_ATMP_MODE_ON_HOST:{
        __setupATMPModeOnHost();
        break;
      }
    }
    
  }
};
inline void chagneDeviceMode(){
  switch(devmode){
    case DATE:{
      newdevmode = ATMP;
      break;
    }
    case ATMP:{
      newdevmode = DATE;
      break;
    }
  }
}
inline void handleButton(){
  static bool pressed = false;
  if(digitalRead(BUTTON_PIN) == 0 && !pressed){
    pressed = true;
    chagneDeviceMode();
  }
  if(digitalRead(BUTTON_PIN) == 1){
    pressed = false;
  }
}
MessageController msger{};
void loop() {
  struct dissegs dis = setDisplay(number);
  draw(&dis);
  msger.next();
  handleButton();
}
