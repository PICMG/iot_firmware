#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

int main()
{
    char ch;
    
    // init test
    uart_struct uart1 = { .descriptor=0, .is_connected=0};
    if(uart_init(&uart1,"/dev/ttyUSB0")) printf("connection successful\n");
    
    // write char loopback test
    uart_writeCh(&uart1,'A');
    while(!uart_readCh(&uart1,&ch));
    printf("%c\n",ch);
    
    //write buffer loopback test
    char str;
    uart_writeBuffer(&uart1,"ABCD",4);
    for(int i=0; i<4; i++)
    {
        while(!uart_readCh(&uart1,&str));
        printf("%c",str);
    }printf("\n");

    // close test
    uart_close(&uart1);
    return 0;
}
