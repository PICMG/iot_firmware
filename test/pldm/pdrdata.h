#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define PDR_BYTE_TYPE unsigned char

extern PDR_BYTE_TYPE __pdr_data[];
extern unsigned int __pdr_total_size;
extern unsigned int __pdr_number_of_records;
extern unsigned int __pdr_max_record_size;

#ifdef __cplusplus
}
#endif
