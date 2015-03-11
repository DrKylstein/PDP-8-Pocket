#ifdef DEBUG
#include <cstdio>
#endif
#include "PDP8.hpp"

#define DF_M 00007
#define U_M 00100
#define IE_M 00200
#define II_M 00400
#define IR_M 01000
#define GT_M 02000
  
enum {
    DF_S,
    U_S = 3,
    IE_S,
    II_S,
    IR_S,
    GT_S,
};

enum {
    FETCH_STATE,
    DEFER_STATE,
    EXECUTE_STATE,
    IO_STATE
};

#define HALT_M 010000
#define IF_M 070000
#define LINK_M 0xF000
#define AC_M 07777

#define TTYOUT_M 010
#define TTYIN_M 004
#define TTYIE_M 001

#define Z_M 00200
#define I_M 00400
#define PAGE_M 07600
#define OFFSET_M 00177

void PDP8::reset() {
    _l_ac = 0;
    _if_pc = 0;
    _flags = 0;
    for(int i = 0; i < PDP8_TTY_COUNT; i++) {
        _ttys[i].flags = TTYIE_M;
    }
    _ms = FETCH_STATE;
    _sr &= ~HALT_M;
}

bool PDP8::isInputReady(int id) {
    if(id < 0 || id > PDP8_TTY_COUNT) return false;
    return (_ttys[id].flags & TTYIN_M) == 0;
}
void PDP8::setInput(uint8_t c, int id) {
    if(id < 0 || id > PDP8_TTY_COUNT) return;
    _ttys[id].in = c;
    _ttys[id].flags |= TTYIN_M;
}
bool PDP8::isOutputReady(int id) {
    if(id < 0 || id > PDP8_TTY_COUNT) return false;
    return (_ttys[id].flags & TTYOUT_M) == 0;
}
uint8_t PDP8::getOutput(int id) {
    uint8_t c = _ttys[id].out;
    _ttys[id].flags |= TTYOUT_M;
    return c;
}


bool PDP8::isReaderReady() {
    return (_punchFlags & TTYIN_M) == 0;
}
bool PDP8::isPunchReady() {
    return (_punchFlags & TTYOUT_M) == 0;
}
char PDP8::getPunched() {
    char result = _punchBuf;
    _punchFlags |= TTYOUT_M;
    return result;
}
void PDP8::setRead(char c) {
    _readBuf = c;
    _punchFlags |= TTYIN_M;
    _readRequested = false;
}


void PDP8::setSwitches(int sw) {
    _sr = (_sr & ~07777) | (sw & 07777);
}
void PDP8::loadAddress() {
    _if_pc = _ma = (_if_pc & IF_M) | (_sr & 07777);
}
void PDP8::deposit(bool advance) {
    _mb = _sr & 07777;
    _ram[_ma] = _mb;
    if(advance) _ma = (_if_pc & IF_M) | ((_ma + 1) & 07777);
}
int PDP8::examine(bool advance) {
    _mb = _ram[_ma];
    if(advance) _ma = (_if_pc & IF_M) | ((_ma + 1) & 07777);
    return _mb & 07777;
}

int PDP8::getPC() {
    return _if_pc;
}
int PDP8::getMB() {
    return _mb & 07777;
}
int PDP8::getMA() {
    return _ma;
}
int PDP8::getLAC() {
    return _l_ac;
}
bool PDP8::isHalted() {
    return _sr & HALT_M;
}
int PDP8::getState() {
    return _ms;
}

#define ADVANCE_PC _if_pc = (_if_pc & 070000) | ((_if_pc + 1) & 07777)

void PDP8::step() {
    if(_sr & HALT_M) return;
    
    switch(_ms) {
        case FETCH_STATE:
            //get extended address
            _ma = _if_pc;
        
            #ifdef DEBUG
            printf("%05o %05o", _if_pc, _l_ac & 017777);
            #endif
        
            //get instruction word
            _mb = _ram[_ma];
            _ir = (_mb >> 9) & 7;
        
            ADVANCE_PC;
        
            //decide instruction type
            if(_ir == 6) { //IOT
                if((_mb & 00770)==0) { //CPU op
                    switch(_mb & 7) {
                        case 1: //ION
                            #ifdef DEBUG
                            puts(" ION");
                            #endif
                            _flags |= 1 << IE_S;
                            break;
                        case 2: //IOFF
                            #ifdef DEBUG
                            puts(" IOFF");
                            #endif
                            _flags &= ~IE_S;
                            break;
                        #ifdef PDP8_E
                        case 0: //SKON
                            if(_flags & IE_M) {
                               ADVANCE_PC;
                            }
                            break;
                        case 3: //SRQ
                            if(_flags & IR_M) {
                                ADVANCE_PC;
                            }
                            break;
                        case 4: //GTF
                            //_flags = FLAGS[5] | DF[3]
                            // L[1] | FLAGS[8] | IF[3] | DF[3]
                            _l_ac = (_l_ac & LINK_M) | ((_l_ac & 0x1000) >> 1) | ((_flags & 0xFC) << 3) | ((_if_pc & IF_M) >> 9) | (_flags & 0x07);
                            break;
                        case 5: //RTF
                            _flags = ((_l_ac >> 3) & 0xFC) | (_l_ac & 0x07);
                            _if_pc = (_if_pc & 0x0FFF) | ((_l_ac << 9) & IF_M);
                            _l_ac = (_l_ac & 0x0FFF) | ((_l_ac & 0x0100) << 1);
                            break;
                        case 6: //SGT
                            if(_flags & GT_M)  ADVANCE_PC;
                            break;
                        case 7: //CAF
                            _if_pc &= ~IF_M;
                            _flags = 0;
                            break;
                        #endif
                    }
                } else { //peripheral op
                    _ms = IO_STATE;
                }
            } else if(_ir == 7) { //OPR                
                if((_mb & 00400)==0) { //group 1
                    if(_mb & 00200) {
                        #ifdef DEBUG
                        printf(" CLA");
                        #endif
                        _l_ac &= LINK_M; //CLA
                    }

                    if(_mb & 00100) {
                        #ifdef DEBUG
                        printf(" CLL");
                        #endif
                        _l_ac &= AC_M; //CLL
                    }
                    if(_mb & 00040) {
                        #ifdef DEBUG
                        printf(" CMA");
                        #endif
                        _l_ac ^= AC_M; //CMA
                    }
                    if(_mb & 00020) {
                        #ifdef DEBUG
                        printf(" CML");
                        #endif
                        _l_ac ^= LINK_M; //CML
                    }
                    if(_mb & 00001) {
                        #ifdef DEBUG
                        printf(" IAC");
                        #endif
                        _l_ac++; //IAC
                    }
                    
                    #ifdef DEBUG
                    if(_mb & 00002) {
                        if(_mb & 00010) {
                            printf(" RTR");
                        }
                        if(_mb & 00004) {
                            printf(" RTL");
                        }
                    } else {
                        if(_mb & 00010) {
                            printf(" RAR");
                        }
                        if(_mb & 00004) {
                            printf(" RAL");
                        }
                    }
                    #endif
                    
                    for(int i = 0; i<((_mb & 00002) ? 2 : 1); i++) {
                        if(_mb & 00010) { //RAR / RTR
                            int16_t temp = _l_ac;
                            _l_ac >>= 1;
                            if(temp & 0x0001) _l_ac |= LINK_M;
                            
                        }
                        if(_mb & 00004) { //RAL / RTL
                            int16_t temp = _l_ac;
                            _l_ac <<= 1;
                            if(temp & 0x1000) _l_ac |= 0x0001;
                        }
                    }
                    #ifdef PDP8_E
                    if(_mb == 07002) { //BSW
                        _l_ac = (_l_ac & LINK_M) | ((_l_ac & 00077) << 6) | ((_l_ac & 07700) >> 6);
                    }
                    #endif
                } else if((_mb & 00001)==0) { //group 2
                    #ifdef DEBUG
                        if(_mb & 00010) {
                            if(_mb & 00100) printf(" SPA");
                            if(_mb & 00040) printf(" SNA");
                            if(_mb & 00020) printf(" SZL");
                        } else {
                            if(_mb & 00100 )printf(" SMA");
                            if(_mb & 00040) printf(" SZA");
                            if(_mb & 00020) printf(" SNL");
                        }
                    #endif
                    if(
                        (
                            ((_l_ac&04000) && (_mb&00100)) || //SMA/SPA
                            ((_l_ac & 07777)==0  &&  (_mb & 00040)) ||  //SZA/SNA
                            ((_l_ac&0x1000) && (_mb&00020)) //SNL/SZL
                        ) ^ ((_mb & 00010)!=0) //invert logic based on this bit
                    ) {
                        ADVANCE_PC;
                    }
                    
                    if(_mb & 00200) {
                        #ifdef DEBUG
                        printf(" CLA");
                        #endif
                        _l_ac &= LINK_M; //CLA
                    }

                    if(_mb & 00004) { //OSR
                        #ifdef DEBUG
                        printf(" OSR");
                        #endif
                        //TODO: is trapped in timeharing config
                        _l_ac |= _sr & 07777;
                    }
                    if(_mb & 00002) { //HLT
                        #ifdef DEBUG
                        printf(" HLT");
                        #endif
                        //TODO: is trapped in timeharing config
                        _sr |= HALT_M;
                    }
                } else {
                    if(_mb & 00200) {
                        #ifdef DEBUG
                        printf(" CLA");
                        #endif
                        _l_ac &= LINK_M; //CLA
                    }

                    if(_mb & 00120) { //MQA MQL / SWP
                        #ifdef DEBUG
                        printf(" MQA+MQL");
                        #endif
                        //using mb as temp, not used any further this instruction
                        _mb = _mq; 
                        _mq = (_l_ac & AC_M);
                        _l_ac &= LINK_M;
                        _l_ac |= (_mb & AC_M);
                    } else {
                        if(_mb & 00100) {
                            #ifdef DEBUG
                            printf(" MQA");
                            #endif
                            _l_ac |= (_mq & AC_M); //MQA
                        }
                        if(_mb & 00020) { //MQL
                            #ifdef DEBUG
                            printf(" MQL");
                            #endif
                            _mq = (_l_ac & AC_M);
                            _l_ac &= LINK_M;
                        }
                    }
                }
                #ifdef DEBUG
                    puts("");
                #endif
            } else { //Memory instructions
                #ifdef DEBUG
                switch(_ir) {
                    case 0: //AND
                        printf(" AND");
                        break;
                    case 1: //TAD
                        printf(" TAD");
                        break;
                    case 2: //ISZ
                        printf(" ISZ");
                        break;
                    case 3: //DCA
                        printf(" DCA");
                        break;
                    case 4: //JMS
                        printf(" JMS");
                        break;
                    case 5: //JMP
                        printf(" JMP");
                        break;
                }
                #endif
                _ma &= ~OFFSET_M;
                _ma |= _mb & OFFSET_M;
                //zero page
                if((_mb & Z_M) == 0) {
                    #ifdef DEBUG
                    printf(" Z");
                    #endif
                    _ma &= ~PAGE_M;
                }
                
                if(_mb & I_M) {
                    #ifdef DEBUG
                    printf(" I");
                    #endif
                    _ms = DEFER_STATE;
                } else {
                    _ms = EXECUTE_STATE;
                }
                
                #ifdef DEBUG
                    printf(" %03o\n", _mb & OFFSET_M);
                #endif
            }
            break;
            
        case DEFER_STATE:
            //autoincrement
            _mb = _ram[_ma];
            if((_ma & 07770) == 00010) {
                _mb++;
                _ram[_ma] = _mb & 07777;
            }
            //fetch pointer
            _ma = (_mb & 07777) | ((_flags & DF_M) << 12);
            _ms = EXECUTE_STATE;
            break;
            
        case EXECUTE_STATE:
            _mb = _ram[_ma] & 07777;
            if(_mb & 04000) _mb |= 0xF000;
        
            switch(_ir) {
                case 0: //AND
                    _l_ac &= LINK_M | _mb;
                    break;
                
                case 1: //TAD
                    _l_ac += _mb;
                    break;
                
                case 2: //ISZ
                    _mb++;
                    if(_mb & 0x0FFF == 0) ADVANCE_PC;
                    break;
                
                case 3: //DCA
                    _mb = _l_ac;
                    _l_ac &= LINK_M;
                    break;
                
                case 4: //JMS
                    _mb = _if_pc;
                    //if indirect was used, _ma will be based on DF, force it to IF
                    _if_pc = (_if_pc & IF_M) | ((_ma +1) & 07777);
                    break;
                case 5: //JMP
                    //if indirect was used, _ma will be based on DF, force it to IF
                    _if_pc = (_if_pc & IF_M) | (_ma & 07777);
                    break;
            }
            _ram[_ma] = _mb & 07777;
            _ms = FETCH_STATE;
            break;
            
        case IO_STATE:
            switch((_mb >> 3) & 077) {
                case 001: //high-speed tape reader
                    #ifdef PDP8_E
                    if((_mb & 7)==0) _punchFlags |= 1; //RPE
                    #endif
                    if((_mb&1) && (_punchFlags&TTYIN_M)) ADVANCE_PC; //RSF
                    if(_mb&2) { //RRB
                        _l_ac |= (uint8_t)_readBuf;
                        _punchFlags &= ~TTYIN_M;
                    }
                    if(_mb&4) { //RFC
                        _punchFlags &= ~TTYIN_M;
                        _readRequested = true;
                    }
                    break;
                case 002: //high-speed tape punch
                    #ifdef PDP8_E
                    if((_mb & 7)==0) _punchFlags &= ~1; //PCE
                    #endif
                    if((_mb&1) && (_punchFlags&TTYOUT_M)) ADVANCE_PC; //PSF
                    if(_mb&2) _punchFlags &= ~TTYOUT_M; //PCF
                    if(_mb&4) _punchBuf = _l_ac; //PPC
                break;

                //console tty
                case 003:
                    _doTtyInOp(0);
                    break;
                case 004:
                    _doTtyOutOp(0);
                    break;
                //second tty
                case 040:
                    _doTtyInOp(1);
                    break;
                case 041:
                    _doTtyOutOp(1);
                    break;
                //serial printer
                case 065:
                    _doTtyInOp(2);
                    break;
                case 066:
                    _doTtyOutOp(2);
                    break;
                //VT78 #1
                case 030:
                    _doTtyInOp(3);
                    break;
                case 031:
                    _doTtyOutOp(3);
                    break;
                //VT78 #2
                case 032:
                    _doTtyInOp(4);
                    break;
                case 033:
                    _doTtyOutOp(4);
                    break;

            }
            #ifdef DEBUG
                puts("");
            #endif
        
            _ms = FETCH_STATE;
        
            break;
    }
}

void PDP8::_doTtyInOp(int id) {
    if(id > PDP8_TTY_COUNT) return;
    #ifdef PDP8_E
    if((_mb & 7)==0) _ttys[id].flags &= ~TTYIN_M; //KCF
    if((_mb & 7)==5) { //KIE
        _ttys[id].flags = (_ttys[id].flags & ~3) | (_l_ac & 3);
    } else {
    #endif
    if(_mb&1) {
        #ifdef DEBUG
            printf(" KSF(%d)", id);
        #endif
        if(_ttys[id].flags&TTYIN_M) {
            ADVANCE_PC; //KSF
        }
    }
    if(_mb&2) { //KCC
        #ifdef DEBUG
            printf(" KCC(%d)", id);
        #endif
        _ttys[id].flags &= ~TTYIN_M;
        _l_ac &= LINK_M;
    }
    if(_mb&4) {
        #ifdef DEBUG
            printf(" KRS(%d)", id);
        #endif
        _l_ac |= _ttys[id].in; //KRS
    }
    #ifdef PDP8_E
    }
    #endif
}
void PDP8::_doTtyOutOp(int id) {
    if(id > PDP8_TTY_COUNT) return;
    #ifdef PDP8_E
    if((_mb & 7)==0) _ttys[id].flags |= TTYOUT_M; //TFL
    if((_mb & 7)==5) { //TSK
        if(_ttys[id].flags&014) ADVANCE_PC;
    } else {
    #endif
    if(_mb&1) {
        #ifdef DEBUG
            printf(" TSF(%d)", id);
        #endif
        if(_ttys[id].flags&TTYOUT_M) ADVANCE_PC; //TSF
    }
    if(_mb&2) {
        #ifdef DEBUG
            printf(" TCF(%d)", id);
        #endif
        _ttys[id].flags &= ~TTYOUT_M; //TCF
    }
    if(_mb&4) {
        #ifdef DEBUG
            printf(" TPC(%d)", id);
        #endif
        _ttys[id].out = _l_ac; //TPC
    }
    #ifdef PDP8_E
    }
    #endif
}

