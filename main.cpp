#include <cstdio>
#include <fstream>
#include <chrono>
#include <thread>
#include "PDP8.hpp"

PDP8 pdp;

int main() {
    pdp.reset();
    
    pdp.setSwitches(07756);
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
    pdp.loadAddress();
    std::ifstream tape("binloader.rim", std::ios_base::in | std::ios_base::binary);
    while(!pdp.isHalted()) {
        pdp.step();
        if(pdp.isKeyReady()) {
            int c = tape.get();
            #ifdef DEBUG
            printf("Tape feed %03o\n", c & 0777);
            #endif
            if(c < 0) break;
            pdp.setKey(c);
        }
    }
    tape.close();
    
    puts("Switching tapes...");
    tape.open("hello.bin", std::ios_base::in | std::ios_base::binary);
    pdp.reset();
    pdp.setSwitches(07777);
    pdp.loadAddress();
    while(!pdp.isHalted()) {
        pdp.step();
        if(pdp.isKeyReady()) {
            int c = tape.get();
            #ifdef DEBUG
            printf("Tape feed %03o\n", c & 0777);
            #endif
            if(c < 0) break;
            pdp.setKey(c);
        }
    }
    tape.close();
    
    puts("Executing...");
    pdp.reset();
    pdp.setSwitches(00200);
    pdp.loadAddress();
    while(!pdp.isHalted()) {
        pdp.step();
        if(pdp.isPrintReady()) {
            int c = pdp.getPrinted();
            if(c) putchar(c & 0177);
        }
    }

    
    /*pdp.setSwitches(0);
    pdp.loadAddress();
    for(int i = 0; i < 4096; i++) {
        printf("%04o ", pdp.examine(true));
        if(i % 16 == 15) puts("");
    }*/
    return 0;
}
