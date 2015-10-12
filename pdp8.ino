#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "PDP8.h"

class PDP8FRAM: public PDP8Memory {
    public:
        void begin(int lowerAddr, int upperAddr) {
            lower.begin(lowerAddr);
            upper.begin(upperAddr);
        }
        
        uint16_t get(int addr) {
            return lower.read8(addr & 077777) | (upper.read8(addr & 077777) << 8);
        }
        void set(int addr, uint16_t data) {
            lower.write8(addr & 077777, data);
            upper.write8(addr & 077777, data >> 8);
        }
    private:
        Adafruit_FRAM_I2C lower;
        Adafruit_FRAM_I2C upper;
};

PDP8FRAM ram;
PDP8 pdp(&ram);
int inByte = 0;
Adafruit_8x8matrix left = Adafruit_8x8matrix();
Adafruit_8x8matrix right = Adafruit_8x8matrix();

void drawPixel(uint16_t x, uint16_t y, boolean state) {
    if(x < 8) {
        left.drawPixel(x, y, state? LED_ON : LED_OFF);
    } else {
        right.drawPixel(x-8, y, state? LED_ON : LED_OFF);
    }
}

void setup() {
    Serial.begin(115200);
    /*pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(13, OUTPUT);*/
  
    left.begin(0x71);
    right.begin(0x70);
    ram.begin(0x50, 0x51);
  
    pdp.reset();
    
    pdp.setSwitches(00200);
    pdp.loadAddress();
    pdp.setSwitches(07200); //CLA
    pdp.deposit(true);
    pdp.setSwitches(01000); //TAD 0000
    pdp.deposit(true);
    pdp.setSwitches(07001); //IAC
    pdp.deposit(true);
    pdp.setSwitches(03000); //DCA 0000
    pdp.deposit(true);
    pdp.setSwitches(01000); //TAD 0000
    pdp.deposit(true);
    pdp.setSwitches(07402); //HALT
    pdp.deposit(true);
    pdp.setSwitches(00200);
    pdp.loadAddress();
    
    /*pdp.setSwitches(00200);
    pdp.loadAddress();
    pdp.setSwitches(07200);
    pdp.deposit(true);
    pdp.setSwitches(07100);
    pdp.deposit(true);
    pdp.setSwitches(01220);
    pdp.deposit(true);
    pdp.setSwitches(03010);
    pdp.deposit(true);
    pdp.setSwitches(07000);
    pdp.deposit(true);
    pdp.setSwitches(01410);
    pdp.deposit(true);
    pdp.setSwitches(07450);
    pdp.deposit(true);
    pdp.setSwitches(07402);
    pdp.deposit(true);
    pdp.setSwitches(04212);
    pdp.deposit(true);
    pdp.setSwitches(05204);
    pdp.deposit(true);
    pdp.setSwitches(00000);
    pdp.deposit(true);
    pdp.setSwitches(06046);
    pdp.deposit(true);
    pdp.setSwitches(06041);
    pdp.deposit(true);
    pdp.setSwitches(05214);
    pdp.deposit(true);
    pdp.setSwitches(07200);
    pdp.deposit(true);
    pdp.setSwitches(05612);
    pdp.deposit(true);
    pdp.setSwitches(00220);
    pdp.deposit(true);
    pdp.setSwitches(00310);
    pdp.deposit(true);
    pdp.setSwitches(00305);
    pdp.deposit(true);
    pdp.setSwitches(00314);
    pdp.deposit(true);
    pdp.setSwitches(00314);
    pdp.deposit(true);
    pdp.setSwitches(00317);
    pdp.deposit(true);
    pdp.setSwitches(00240);
    pdp.deposit(true);
    pdp.setSwitches(00327);
    pdp.deposit(true);
    pdp.setSwitches(00317);
    pdp.deposit(true);
    pdp.setSwitches(00322);
    pdp.deposit(true);
    pdp.setSwitches(00314);
    pdp.deposit(true);
    pdp.setSwitches(00304);
    pdp.deposit(true);
    pdp.setSwitches(00241);
    pdp.deposit(true);
    pdp.setSwitches(0);
    pdp.deposit(true);
    
    pdp.setSwitches(00200);
    pdp.loadAddress();*/
    
    /*pdp.setSwitches(07756);
    pdp.loadAddress();
    
    pdp.setSwitches(06032);
    pdp.deposit(true);
    pdp.setSwitches(06031);
    pdp.deposit(true);
    pdp.setSwitches(05357);
    pdp.deposit(true);
    pdp.setSwitches(06036);
    pdp.deposit(true);
    pdp.setSwitches(07106);
    pdp.deposit(true);
    pdp.setSwitches(07006);
    pdp.deposit(true);
    pdp.setSwitches(07510);
    pdp.deposit(true);
    pdp.setSwitches(05357);
    pdp.deposit(true);
    pdp.setSwitches(07006);
    pdp.deposit(true);
    pdp.setSwitches(06031);
    pdp.deposit(true);
    pdp.setSwitches(05367);
    pdp.deposit(true);
    pdp.setSwitches(06034);
    pdp.deposit(true);
    pdp.setSwitches(07420);
    pdp.deposit(true);
    pdp.setSwitches(03776);
    pdp.deposit(true);
    pdp.setSwitches(03376);
    pdp.deposit(true);
    pdp.setSwitches(05356);
    pdp.deposit(true);
    pdp.setSwitches(0);
    pdp.deposit(true);
    pdp.setSwitches(0);
    pdp.deposit(true);
    pdp.setSwitches(07756);
    pdp.loadAddress();*/
    
    
}

int c;

void loop() {
    //digitalWrite(13, LOW);
    /*if(digitalRead(3)) {
        pdp.reset();
        pdp.setSwitches(07777);
        pdp.loadAddress();
        Serial.println("Entered Bin loader");
    }
    if(digitalRead(4)) {
        pdp.reset();
        pdp.setSwitches(00200);
        pdp.loadAddress();
        digitalWrite(13, HIGH);
        Serial.println("Entered hello");
    }
    if(digitalRead(2)) {*/
        if(Serial.available() && pdp.isInputReady()) {
            c = Serial.read();
            pdp.setInput(c);
            Serial.println(c, 8);
        }
        if(pdp.isOutputReady()) {
            c = pdp.getOutput();
            if(c > 0) {
                Serial.write(c & 0177);
                digitalWrite(13, HIGH);
            }
        }
        pdp.step();
        right.clear();
        left.clear();
        for(int i = 0; i < 13; ++i) {
            drawPixel(12-i,0,(pdp.getPC() & (1 << i))? LED_ON : LED_OFF);
            drawPixel(12-i,1,(pdp.getMA() & (1 << i))? LED_ON : LED_OFF);
            drawPixel(12-i,2,(pdp.getMB() & (1 << i))? LED_ON : LED_OFF);
            drawPixel(12-i,3,(pdp.getLAC() & (1 << i))? LED_ON : LED_OFF);
            drawPixel(12-i,4,(pdp.getMQ() & (1 << i))? LED_ON : LED_OFF);
            drawPixel(12-i,5,(((pdp.getIF() << 3) | pdp.getDF()) & (1 << i))? LED_ON : LED_OFF);
        }
        drawPixel(14,pdp.getInstruction(),LED_ON);
        drawPixel(15,pdp.getState(),LED_ON);
        right.writeDisplay();
        left.writeDisplay();
        delay(100);
    //}
}
