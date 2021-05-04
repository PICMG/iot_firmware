#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

int main(int argc, char *argv[])
{
    char ch;

    if (argc!=2) {
        printf("invalid number of arguments.\n");
        printf("syntax: uart_c_test port\n");
        return -1;       
    }

    // init test
    int uart1_handle;
    uart1_handle = uart_init("/dev/ttyUSB0");
    if (uart1_handle>0) printf("connection successful\n");
    
    // write char loopback test
    uart_writeCh(uart1_handle,'A');
    while(!uart_readCh(uart1_handle,&ch));
    printf("%c\n",ch);
    
    //write buffer loopback test
    char str;
    uart_writeBuffer(uart1_handle,"ABCD",4);
    for(int i=0; i<4; i++)
    {
        while(!uart_readCh(uart1_handle,&str));
        printf("%c",str);
    }printf("\n");

    // close test
    uart_close(uart1_handle);
    return 0;
}
