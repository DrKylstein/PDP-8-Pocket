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

#define DEBOUNCE_DEPTH 50
#define DEBOUNCE_RATE 1000
#define CPU_RATE 2
elapsedMicros sinceInput, sinceCPU;
uint32_t switchHistory[DEBOUNCE_DEPTH];
uint32_t switchHighMask, switchLowMask, switchNow, switchPrev;
int character;
uint8_t buttons;


void setup() {
    Serial.begin(115200);
  
    for(int i = 1; i <= 12; ++i) {
        pinMode(i,INPUT_PULLUP);
    }
    
    for(int i = 14; i <= 17; ++i) {
        pinMode(i,INPUT_PULLUP);
    }
    for(int i = 20; i <= 23; ++i) {
        pinMode(i,INPUT_PULLUP);
    }
    
    switchNow = switchPrev = 0xFFFFFFFF;
    for(int i = 0; i < DEBOUNCE_DEPTH; ++i) {
        switchHistory[i] = 0xFFFFFFFF;
    }
    
    left.begin(0x70);
    right.begin(0x71);
    ram.begin(0x50, 0x51);
}

boolean buttonTriggered(uint32_t mask) {
    return (switchNow & mask && !(switchPrev & mask));
}

void loop() {    
    if(sinceInput > DEBOUNCE_RATE) {
        switchHighMask = 0xFFFFFFFF;
        switchLowMask = 0;
        for(int i = 1; i < DEBOUNCE_DEPTH; ++i) {
            switchHistory[i] = switchHistory[i-1];
            switchHighMask &= switchHistory[i];
            switchLowMask |= switchHistory[i];
        }
        switchHistory[0] = 0;
        for(int i = 1; i <= 12; ++i) {
            switchHistory[0] <<= 1;
            switchHistory[0] |= digitalRead(i);
        }
        for(int i = 23; i >= 20; --i) {
            switchHistory[0] <<= 1;
            switchHistory[0] |= digitalRead(i);
        }
        for(int i = 17; i >= 14; --i) {
            switchHistory[0] <<= 1;
            switchHistory[0] |= digitalRead(i);
        }
        switchHighMask &= switchHistory[0];
        switchLowMask |= switchHistory[0];
        
        switchPrev = switchNow;
        switchNow |= switchHighMask;
        switchNow &= switchLowMask;
        sinceInput = 0;
    }
    
    pdp.setSwitches((~switchNow >> 8) & 07777);
    if(buttonTriggered(1)) {
        pdp.deposit(true);
    }
    if(buttonTriggered(2)) {
        pdp.singleStep();
    }
    if(buttonTriggered(4)) {
        pdp.halt();
    }
    if(buttonTriggered(8)) {
        pdp.examine(true);
    }
    if(buttonTriggered(16)) {
        pdp.resume();
    }
    if(buttonTriggered(32)) {
        pdp.start();
    }
    if(buttonTriggered(64)) {
        pdp.setFields((switchNow >> 3) & 7, switchNow & 7);
    }
    if(buttonTriggered(128)) {
        pdp.loadAddress();
    }

    if(Serial.available() && pdp.isInputReady()) {
        character = Serial.read();
        pdp.setInput(character);
        Serial.println(character, 8);
    }
    if(pdp.isOutputReady()) {
        character = pdp.getOutput();
        if(character > 0) {
            Serial.write(character & 0177);
            digitalWrite(13, HIGH);
        }
    }

    if(sinceCPU > CPU_RATE) {
        pdp.cycle();
        sinceCPU = 0;
    }
    
    right.clear();
    left.clear();
    for(int i = 0; i < 13; ++i) {
        drawPixel(15-i,0,(pdp.getPC() & (1 << i))? LED_ON : LED_OFF);
        drawPixel(15-i,1,(pdp.getMA() & (1 << i))? LED_ON : LED_OFF);
        drawPixel(15-i,2,(pdp.getMB() & (1 << i))? LED_ON : LED_OFF);
        drawPixel(15-i,3,(pdp.getLAC() & (1 << i))? LED_ON : LED_OFF);
        drawPixel(15-i,4,(pdp.getMQ() & (1 << i))? LED_ON : LED_OFF);
        drawPixel(15-i,5,(((pdp.getIF() << 3) | pdp.getDF()) & (1 << i))? LED_ON : LED_OFF);
        //drawPixel(15-i,6,((switchNow >> 12) & (1 << i))? LED_ON : LED_OFF);
        //drawPixel(15-i,7,((switchNow & 07777) & (1 << i))? LED_ON : LED_OFF);
    }
    drawPixel(pdp.getInstruction(),6,LED_ON);
    drawPixel(pdp.getState(),7,LED_ON);
    right.writeDisplay();
    left.writeDisplay();
}
