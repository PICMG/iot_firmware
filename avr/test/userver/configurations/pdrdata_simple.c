#include "pdrdata.h"

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
   0x05, 0x00, 0x00, 0x00, 0x01, 0x08, 0x01, 0x00, 0x60, 0x00, 0x01, 0x00, 0x00, 0x80, 0x5a, 0x31, 
   0x00, 0x00, 0x01, 0x00, 0x01, 0x02, 0x01, 0x01, 0x01, 0x65, 0x6e, 0x00, 0x00, 0x54, 0x00, 0x72, 
   0x00, 0x69, 0x00, 0x67, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x41, 0x00, 0x63, 0x00, 0x74, 
   0x00, 0x69, 0x00, 0x76, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x64, 0x00, 0x00, 0x02, 0x02, 
   0x01, 0x65, 0x6e, 0x00, 0x00, 0x54, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x67, 0x00, 0x65, 
   0x00, 0x72, 0x00, 0x44, 0x00, 0x65, 0x00, 0x61, 0x00, 0x63, 0x00, 0x74, 0x00, 0x69, 0x00, 0x76, 
   0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x64, 0x00, 0x00, 
   // State Sensor GlobalInterlockSensor
   0x06, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 
   // State Effecter GlobalInterlockEffecter
   0x07, 0x00, 0x00, 0x00, 0x01, 0x0b, 0x01, 0x00, 0x13, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x60, 0x00, 0x01, 0x03, 
   // State Sensor TriggerSensor
   0x08, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x01, 0x03, 
   // State Effecter TriggerEffecter
   0x09, 0x00, 0x00, 0x00, 0x01, 0x0b, 0x01, 0x00, 0x13, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x01, 0x03, 
   // Numeric Sensor Sensor1
   0x0a, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x5f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 
   0x05, 0x00, 0x00, 0x80, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x3f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0x4a, 0xbc, 0xfa, 0xfd, 
   0x4a, 0xbc, 0xfa, 0x05, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 
   0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 
   0x00, 0x2d, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 
   // State Sensor Sensor2
   0x0b, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x60, 
   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x43, 0x00, 0x01, 0x03
};

FRU_BYTE_TYPE __fru_data[] FRU_DATA_ATTRIBUTES = {
   // FRU Record 1
   0x01, 0x00, 0x01, 0x01, 0x02, 0x06, 0x50, 0x49, 0x43, 0x4d, 0x47, 0x00
};

LINTABLE_TYPE __lintable_ain2_5v[] LINTABLE_DATA_ATTRIBUTES = { 
   0x80000002, 0x9e0d777f, 0xfabc4afd, 0xffd5786b, 0xffe0a928, 0xffe78d58, 0xffecba7a, 0xfff0f07c, 
   0xfff48768, 0xfff7af5d, 0xfffa8563, 0xfffd1cad, 0xffff8268, 0x0001c05b, 0x0003dd86, 0x0005df93, 
   0x0007cafc, 0x0009a2f4, 0x000b6ab7, 0x000d24a9, 0x000ed279, 0x0010762b, 0x0012115a, 0x0013a537, 
   0x00153334, 0x0016bc3a, 0x0018410e, 0x0019c2d7, 0x001b42de, 0x001cc181, 0x001e3f63, 0x001fbe18, 
   0x00213d24, 0x0022be87, 0x002441f3, 0x0025c9a5, 0x002754d8, 0x0028e54a, 0x002a7ac8, 0x002c194f, 
   0x002dbeef, 0x002f6d33, 0x00312682, 0x0032eb3b, 0x0034bde2, 0x00369e01, 0x00389002, 0x003a9643, 
   0x003cae57, 0x003ee3a0, 0x00413801, 0x0043aad3, 0x00464403, 0x00490931, 0x004c0713, 0x004f4302, 
   0x0052ced3, 0x0056b3c7, 0x005b1a10, 0x00601cc7, 0x0065e799, 0x006ceb9d, 0x0075aacf, 0x0080ffca, 
   0x008ca9df, 0x0097f458, 0x00a31606, 0x00ae37b3, 0x00b95961
};
