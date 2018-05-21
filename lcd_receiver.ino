#include <FastGPIO.h>
#define FILLARRAY(a,n) a[0]=n, memcpy( ((char*)a)+sizeof(a[0]), a, sizeof(a)-sizeof(a[0]) );

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define RS 12
#define EN 11
#define RW 10
#define D7 2
#define D6 3
#define D5 4
#define D4 5
#define D3 6
#define D2 7
#define D1 8
#define D0 9
volatile char d0,d1,d2,d3,d4,d5,d6,d7,var;

#define MODE_CMD 1
#define MODE_DATA 2

//Offsets
#define OFFSET_R1 0x40
void setup() {
  Serial.begin(115200);
  FastGPIO::Pin<RS>::setInput();    //Define RS
  FastGPIO::Pin<EN>::setInput();    //Define EN
  FastGPIO::Pin<RW>::setInput();    //Define R/W
  FastGPIO::Pin<D7>::setInput();
  FastGPIO::Pin<D6>::setInput(); 
  FastGPIO::Pin<D5>::setInput();
  FastGPIO::Pin<D4>::setInput();
  FastGPIO::Pin<D3>::setInput();
  FastGPIO::Pin<D2>::setInput();
  FastGPIO::Pin<D1>::setInput();
  FastGPIO::Pin<D0>::setInput();
}

inline char readBytes(){
   d7=FastGPIO::Pin<D7>::isInputHigh() ? 1 : 0;
   d6=FastGPIO::Pin<D6>::isInputHigh() ? 1 : 0;
   d5=FastGPIO::Pin<D5>::isInputHigh() ? 1 : 0;
   d4=FastGPIO::Pin<D4>::isInputHigh() ? 1 : 0;
   d3=FastGPIO::Pin<D3>::isInputHigh() ? 1 : 0;
   d2=FastGPIO::Pin<D2>::isInputHigh() ? 1 : 0;
   d1=FastGPIO::Pin<D1>::isInputHigh() ? 1 : 0;
   d0=FastGPIO::Pin<D0>::isInputHigh() ? 1 : 0;
   var = (d7<<7)+(d6<<6)+(d5<<5)+(d4<<4)+(d3<<3)+(d2<<2)+(d1<<1)+(d0);
   return var;
}

char cmdBuff[128];
char msgbuff[32];
volatile long previousMillis = 0;        // will store last time LED was updated
volatile long interval = 5000;           // interval at which to blink (milliseconds)
volatile char si=0;

inline void handleCmd(){
  char cmd = readBytes();
  bool handled = false;
  bool hasMsg = true;
  if(cmd==LCD_CLEARDISPLAY){
     handled = true;
     si=0;
     FILLARRAY(msgbuff, 0);
     //sprintf(cmdBuff, "clr");
     hasMsg = false;
  }else if(cmd & LCD_SETDDRAMADDR){
    char col = cmd ^ LCD_SETDDRAMADDR;
    char row = 0;
    if(col>OFFSET_R1){ col = col - OFFSET_R1; row = 1; }
    //sprintf(cmdBuff, "Set col to: r:%d c:%d", row, col);
    handled = true;
  }
  if(handled && hasMsg){
    Serial.println(cmdBuff);
  }else if(!handled){
    sprintf(cmdBuff, "Unknown command: %d", cmd);
    Serial.println(cmdBuff);
  }
}

inline void handleData(){
  if(si>16) si=0;
  char rcv = readBytes();
  msgbuff[si++] = rcv;
  //char dt[10];
  //sprintf(dt, "%d %c p:%d", rcv, rcv, si);
  //Serial.println(dt);
  if(si>=3){
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {  
      Serial.println(msgbuff);
    }  
  }  
}

bool low=true;
void loop() {
  bool crlow = !FastGPIO::Pin<EN>::isInputHigh();
  if(!low && crlow){
    char mode = !FastGPIO::Pin<RS>::isInputHigh() ? MODE_CMD : MODE_DATA;
    bool isWrite = !FastGPIO::Pin<RW>::isInputHigh();
    if(isWrite){
      if(mode==MODE_CMD){
        handleCmd();
      }else if(mode==MODE_DATA){
        handleData();
      }else{
        Serial.println("Invalid mode..");
      }
    }
  }  
  low = crlow;
}





