//Description: Header file for Frame Check Sequence
//Authors: Douglas Sandy, David Sandy
//Copyright 2020 PICMG all rights reserved

#pragma once

#include <iostream>

class FrameCheckSequence {
private:

public:
    //constant values used as checks by the FCS
    static const unsigned int INITFCS = 0xffff;
    static const unsigned int GOODFCS = 0xf0b8;
    //FCS lookup table as calculated by the table generator.
    static const unsigned int fcstab[256];
    //FCS value generator
    unsigned int calcFcs(unsigned int fcs, unsigned char* cp, unsigned int len);
};

