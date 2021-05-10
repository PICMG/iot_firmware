#!/bin/bash
# install tools required to build the guest additions
cd ~
sudo apt install gcc make git
sudo apt-get install build-essential g++ libtool-bin binutils-dev texinfo autoconf swig
git clone https://github.com/Traumflug/simulavr.git
perl -i -0pe 's/ \*timeToNextStepIn_ns = \(SystemClockOffset\)1000000;/ *timeToNextStepIn_ns = (SystemClockOffset)1000000;\n    return 0;/' ./simulavr/src/ui/serialtx.cpp
cd simulavr
./bootstrap
./configure --disable-doxygen-doc --enable-dependency-tracking
make
cd ~
