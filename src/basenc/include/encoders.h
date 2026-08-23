#ifndef ENCODERS_H
#define ENCODERS_H

#include "decoders.h"
#include <stddef.h>

extern const char base64_encoding_table[];
extern const char base64url_encoding_table[];
extern const char base58_alphabet[];
extern const char base32_alphabet[];
extern const char base32hex_alphabet[];
extern const char base16_alphabet[];
extern const char z85_alphabet[];

char* base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base64url_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base58_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base32_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base32hex_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base16_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base2msbf_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* base2lsbf_encode(const unsigned char *data, size_t input_length, size_t *output_length);
char* z85_encode(const unsigned char *data, size_t input_length, size_t *output_length);

#endif
