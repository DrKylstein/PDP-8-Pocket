#include <cstdio>
#include <fstream>
#include "PDP8.hpp"

PDP8 pdp;

int main() {
    pdp.reset();
    
    /*pdp.setSwitches(00010);
    pdp.loadAddress();
    pdp.setSwitches(00200);
    pdp.deposit();
    pdp.loadAddress();
    pdp.setSwitches('H');
    pdp.deposit(true);
    pdp.setSwitches('e');
    pdp.deposit(true);
    pdp.setSwitches('l');
    pdp.deposit(true);
    pdp.deposit(true);
    pdp.setSwitches('o');
    pdp.deposit(true);
    pdp.setSwitches(' ');
    pdp.deposit(true);
    pdp.setSwitches('f');
    pdp.deposit(true);
    pdp.setSwitches('r');
    pdp.deposit(true);
    pdp.setSwitches('o');
    pdp.deposit(true);
    pdp.setSwitches('m');
    pdp.deposit(true);
    pdp.setSwitches(' ');
    pdp.deposit(true);
    pdp.setSwitches('1');
    pdp.deposit(true);
    pdp.setSwitches('9');
    pdp.deposit(true);
    pdp.setSwitches('6');
    pdp.deposit(true);
    pdp.setSwitches('5');
    pdp.deposit(true);
    pdp.setSwitches('!');
    pdp.deposit(true);
    pdp.setSwitches('\n');
    pdp.deposit(true);
    
    pdp.setSwitches(00400);
    pdp.loadAddress();
    pdp.setSwitches(07200); //0400 CLA
    pdp.deposit(true);
    pdp.setSwitches(06046); //0401 TLS
    pdp.deposit(true);
    pdp.setSwitches(06041); //0402 TSF
    pdp.deposit(true);
    pdp.setSwitches(05202); //0403 JMP 0402
    pdp.deposit(true);
    pdp.setSwitches(07200); //0404 CLA
    pdp.deposit(true);
    pdp.setSwitches(01410); //0405 TAD I Z 010
    pdp.deposit(true);
    pdp.setSwitches(07450); //0406 SNA
    pdp.deposit(true);
    pdp.setSwitches(05214); //0407 JMP 0414
    pdp.deposit(true);
    pdp.setSwitches(06046); //0410 TLS
    pdp.deposit(true);
    pdp.setSwitches(06041); //0411 TSF
    pdp.deposit(true);
    pdp.setSwitches(05211); //0412 JMP 0411
    pdp.deposit(true);
    pdp.setSwitches(05204); //0413 JMP 0404
    pdp.deposit(true);
    pdp.setSwitches(07402); //0414 HLT
    pdp.deposit(true);
    
    pdp.setSwitches(00400);
    pdp.loadAddress();*/
    
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


/*	pdp.setSwitches(01000);
	pdp.loadAddress();
	
	pdp.setSwitches(07240);
	pdp.deposit(true);
	pdp.setSwitches(03400);
	pdp.deposit(true);
	pdp.setSwitches(07402);
	pdp.deposit(true);
	
	pdp.setSwitches(0);
	pdp.loadAddress();
	pdp.setSwitches(00200);
	pdp.deposit();
	
	pdp.setSwitches(01000);
	pdp.loadAddress();*/

    std::ifstream tape("binloader.rim", std::ios_base::in | std::ios_base::binary);

    while(!pdp.isHalted()) {
        //printf("MS%d MA%05o MB%04o LAC%05o\n",pdp.getState(), pdp.getMA(), pdp.getMB(), pdp.getLAC() & 017777);
        pdp.step();
        if(pdp.isKeyReady()) {
            int c = tape.get();
            //printf("<Tape feed: %02o>\n", c);
            if(c < 0) break;
            pdp.setKey(c);
        }
        if(pdp.isPrintReady()) {
            //printf("TTY: '%c'\n", pdp.getPrinted());
        }
    }

    tape.close();
    tape.open("hello.bin", std::ios_base::in | std::ios_base::binary);

    pdp.reset();
    pdp.setSwitches(07756);
    pdp.loadAddress();

    while(!pdp.isHalted()) {
        printf("MS%d MA%05o MB%04o LAC%05o\n",pdp.getState(), pdp.getMA(), pdp.getMB(), pdp.getLAC() & 017777);
        pdp.step();
        if(pdp.isKeyReady()) {
            int c = tape.get();
            printf("<Tape feed: %02o>\n", c);
            if(c < 0) break;
            pdp.setKey(c);
        }
        if(pdp.isPrintReady()) {
            printf("TTY: '%c'\n", pdp.getPrinted());
        }
    }

    pdp.setSwitches(0);
    pdp.loadAddress();
    for(int i = 0; i < 4096; i++) {
        printf("%04o ", pdp.examine(true));
        if(i % 16 == 15) puts("");
    }
    return 0;
}
