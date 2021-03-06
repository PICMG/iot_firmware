//*****************************************************************
// config.c
//
// This file was autogenerated by the PICMG firmware builder
// utility.  It includes data structures used to build a custom
// configured firmware image.
#include "config.h"

PDR_BYTE_TYPE __pdr_data[] PDR_DATA_ATTRIBUTES = { 
   // Terminus Locator PDR 
   0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x09, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 
   0x01, 0x01, 0x01, 
   // FRU Record Set 
   0x02, 0x00, 0x00, 0x00, 0x01, 0x14, 0x01, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x00, 0x50, 0x00, 
   0x01, 0x00, 0x00, 0x00, 
   // Entity Association 
   0x03, 0x00, 0x00, 0x00, 0x01, 0x0f, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x50, 0x00, 0x01, 
   0x00, 0x00, 0x00, 0x01, 0x00, 0x60, 0x01, 0x00, 0x01, 0x00, 
   // OEM Entity ID 
   0x04, 0x00, 0x00, 0x00, 0x01, 0x11, 0x01, 0x00, 0x1c, 0x00, 0x01, 0x00, 0x00, 0x60, 0x5a, 0x31, 
   0x00, 0x00, 0x01, 0x00, 0x01, 0x65, 0x6e, 0x00, 0x00, 0x53, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x70, 
   0x00, 0x6c, 0x00, 0x65, 0x00, 0x00, 
   // OEM State Set 
   0x05, 0x00, 0x00, 0x00, 0x01, 0x08, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00, 0x03, 0x80, 0x5a, 0x31, 
   0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x01, 0x65, 0x6e, 0x00, 0x00, 0x54, 0x00, 0x72, 
   0x00, 0x69, 0x00, 0x67, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x02, 0x02, 0x01, 0x65, 
   0x6e, 0x00, 0x00, 0x54, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 
   0x00, 0x00, 
   // State Sensor GlobalInterlockSensor
   0x06, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 
   // State Sensor TriggerSensor
   0x07, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 
   // Numeric Sensor Sensor1
   0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x5f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 
   0x05, 0x00, 0x00, 0x80, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x3f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x9b, 0xa0, 0x00, 0x83, 
   0x9b, 0xa0, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

FRU_BYTE_TYPE __fru_data[] FRU_DATA_ATTRIBUTES = {
   // FRU Record 1
   0x01, 0x00, 0x01, 0x02, 0x02, 0x07, 0x05, 0x50, 0x49, 0x43, 0x4d, 0x47, 0x0c, 0x05, 0x53, 0x6d, 
   0x61, 0x72, 0x74
};

LINTABLE_TYPE __lintable_ain5v[] LINTABLE_DATA_ATTRIBUTES = { 
   0x00bf1502, 0x00afd843, 0x00a09b84, 0x0091a582, 0x008269f2, 0x0072ac20, 0x00668a96, 0x005d9094, 
   0x005631af, 0x00507e00, 0x004b5b8c, 0x0047330d, 0x00435b3f, 0x003fad15, 0x003c86c2, 0x0039a2c5, 
   0x0036e548, 0x00344e3c, 0x0031e364, 0x002fa8ca, 0x002d93c4, 0x002b93ec, 0x0029a380, 0x0027c452, 
   0x0025f762, 0x00243a49, 0x002288dc, 0x0020dff1, 0x001f408d, 0x001dac72, 0x001c249e, 0x001aa7aa, 
   0x001932de, 0x0017c293, 0x00165607, 0x0014ede8, 0x00138ac0, 0x00122c3e, 0x0010d08c, 0x000f74b7, 
   0x000e1475, 0x000cae56, 0x000b4467, 0x0009da35, 0x0008730a, 0x00070d85, 0x0005a66f, 0x00043836, 
   0x0002bef3, 0x00013b49, 0xffffaf39, 0xfffe1a68, 0xfffc7745, 0xfffac059, 0xfff8f14b, 0xfff70f15, 
   0xfff5205a, 0xfff31157, 0xfff0c031, 0xffee277c, 0xffeb4883, 0xffe42b54, 0xffda60dd, 0xffd59779, 
   0xffc8caae, 0xff665de7, 0xfed874c2, 0xfe51180a, 0xfdc9bb52
};
