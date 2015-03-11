#include <cstdio>
#include <fstream>
#include <chrono>
#include <thread>
#include "PDP8.hpp"

void bootstrap(PDP8& pdp, const char* filename) {
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
        if(pdp.isInputReady()) {
            int c = tape.get();
            if(c < 0) break;
            pdp.setInput(c);
        }
    }
    tape.close();
    
    tape.open(filename, std::ios_base::in | std::ios_base::binary);
    pdp.reset();
    pdp.setSwitches(07777);
    pdp.loadAddress();
    while(!pdp.isHalted()) {
        pdp.step();
        if(pdp.isInputReady()) {
            int c = tape.get();
            if(c < 0) break;
            pdp.setInput(c);
        }
    }
    tape.close();
}

int main() {
    PDP8 pdp;
    puts("Booting...");
    bootstrap(pdp, "hello.bin");
    
    pdp.reset();
    pdp.setSwitches(0200);
    pdp.loadAddress();
    puts("Running....");
    while(!pdp.isHalted()) {
        pdp.step();
        if(pdp.isOutputReady(1)) {
            int c = pdp.getOutput(1);
            if(c) putchar(c & 0177);
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    puts("\nDone.");
    return 0;
}
