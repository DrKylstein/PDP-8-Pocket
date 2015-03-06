#ifndef PDP8_HPP
#define PDP8_HPP
#include <cstdint>

#ifndef PDP8_RAM_SIZE
#define PDP8_RAM_SIZE 4096
#endif

class PDP8 {
    public:
        void reset();
        void step();
        void setSwitches(int);
        void loadAddress();
        void deposit(bool = false);  
        int examine(bool = false);
    
        int getPC();
        int getMA();
        int getMB();
        int getLAC();
        int getState();
        bool isHalted();
            
        bool isKeyReady();
        bool isPrintReady();
        char getPrinted();
        void setKey(char);

        bool isReaderReady();
        bool isPunchReady();
        char getPunched();
        void setRead(char);
    
    
    private:
        int16_t _ram[PDP8_RAM_SIZE]; //12bits
        int16_t _l_ac; //13 bits
        int16_t _mq; // 12bits
        int16_t _if_pc; //15bits
        int16_t _sr; // halt[1] | sr[12]
        
        int16_t _ma; //15bits
        int16_t _mb; //16 bits
        uint8_t _ir; //3 bits
    
        uint16_t _flags; //8bits
        uint8_t _ms; //2bits
        
        char _ttyIn;
        char _ttyOut;
        uint8_t _ttyFlags; //4 bits
    
        char _readBuf;
        char _punchBuf;
        uint8_t _punchFlags;
        bool _readRequested;
};

#endif
