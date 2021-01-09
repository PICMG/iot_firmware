//*******************************************************************
// calc_new_crc8()
// This function updateas the crc-8 checksum when a new byte is added
// to a message.  CRC-8 is based on the polynomial x^8 + x^2 + x^1 + 1.
// This brute force method will be somewhat slow, but it is only used
// during when getting the PDR repository so it should not impact 
// operational performance.
unsigned char calc_new_crc(unsigned char old_crc, unsigned char new_byte) {
    unsigned crc = old_crc ^ new_byte;
    for (int i = 0; i < 8; i++) crc = (crc << 1) ^ ((crc & 0x80) ? 0x07 : 0);
    return crc;
}
