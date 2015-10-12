#ifndef PDP8_HPP
#define PDP8_HPP
#include <cstdint>

#ifndef PDP8_RAM_SIZE
#define PDP8_RAM_SIZE 4096
#endif

#ifndef PDP8_TTY_COUNT
    #define PDP8_TTY_COUNT 2
#endif

class PDP8Memory {
    public:
        virtual uint16_t get(int addr) = 0;
        virtual void set(int addr, uint16_t data) = 0;
};

class PDP8 {
    public:
        PDP8(PDP8Memory* ram);
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
        int getMQ();
        int getIF();
        int getDF();
        int getState();
        int getInstruction();
        bool isHalted();
            
        bool isOutputReady(int id = 0);
        uint8_t getOutput(int id = 0);
        bool isInputReady(int id = 0);
        void setInput(uint8_t c, int id = 0);

        bool isReaderReady();
        bool isPunchReady();
        char getPunched();
        void setRead(char);
    
    
    private:
        PDP8Memory* _ram; //12bits
        int16_t _l_ac; //13 bits
        int16_t _mq; // 12bits
        int16_t _if_pc; //15bits
        int16_t _sr; // halt[1] | sr[12]
        
        int16_t _ma; //15bits
        int16_t _mb; //16 bits
        uint8_t _ir; //3 bits
    
        uint16_t _flags; //8bits
        uint8_t _ms; //2bits
        
    
        struct Tty {
            uint8_t in, out, flags, id;
        };
    
        Tty _ttys[PDP8_TTY_COUNT];
        
        void _doTtyInOp(int id);
        void _doTtyOutOp(int id);
            
        uint8_t _readBuf;
        uint8_t _punchBuf;
        uint8_t _punchFlags;
        bool _readRequested;
};

#endif
