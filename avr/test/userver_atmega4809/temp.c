#include "uart.h"

void hex4(unsigned char val) {
    if (val>0x9) {
        uart_writeCh('A'+val-10);
    } else {
        uart_writeCh('0'+val);
    }
}

void hex8(unsigned char val) {
    hex4(val>>4);
    hex4(val&0xF);
}

void hex16(unsigned int val) {
    hex8(val>>8);
    hex8(val&0xFF);
}
