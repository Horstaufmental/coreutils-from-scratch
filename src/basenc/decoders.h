#ifndef DECODERS_H
#define DECODERS_H

#include <stddef.h>

extern unsigned char decoding_table[256];
extern unsigned char url_decoding_table[256];
extern unsigned char base58_decoding_table[256];
extern unsigned char base32_decoding_table[256];
extern unsigned char base32hex_decoding_table[256];
extern unsigned char base16_decoding_table[256];
extern unsigned char z85_decoding_table[256];

#define B_64 (1 << 0)
#define B_64URL (1 << 1)
#define B_58 (1 << 2)
#define B_32 (1 << 3)
#define B_32HEX (1 << 4)
#define B_16 (1 << 5)
#define B_2MSB (1 << 6)
#define B_2LSB (1 << 7)
#define B_Z85 (1 << 8)

void init_decode_table_wrapper(unsigned int base);

unsigned char* base64_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base64url_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base58_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base32_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base32hex_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base16_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base2msbf_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* base2lsbf_decode(const char *data, size_t input_length, size_t *output_length);
unsigned char* z85_decode(const char *data, size_t input_length, size_t *output_length);

#endif
