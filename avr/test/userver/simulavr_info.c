#include "simulavr_info.h"

#ifndef F_CPU
#define F_CPU 16000000
#endif

SIMINFO_DEVICE("atmega328");
SIMINFO_CPUFREQUENCY(F_CPU);
//SIMINFO_SERIAL_IN("D0", "-", 9600);  // filename = "-" for stdin
//SIMINFO_SERIAL_OUT("D1", "-", 9600); // filename = "-" for stdout
SIMINFO_SERIAL_IN("D0", "/dev/pts/2", 9600);  // filename = "-" for stdin
SIMINFO_SERIAL_OUT("D1", "/dev/pts/2", 9600); // filename = "-" for stdout
