avr-gcc -Wall -mmcu=atmega328p -DF_CPU=16000000UL -g -I./ -c main.c -o main.o -Wa,-alhds=main.list
avr-g++  -o solution.elf main.o  -mmcu=atmega328p -Wl,-Map=solution.map,--cref libserial.a
avr-objdump -h -S solution.elf > solution.lss
~/simulavr/src/simulavr -n -F 16000000 -d atmega328 -f solution.elf
