#include "decoders.h"
#include "encoders.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

// Base64
unsigned char decoding_table[256];
unsigned char url_decoding_table[256];
// the rest
unsigned char base58_decoding_table[256];
unsigned char base32_decoding_table[256];
unsigned char base32hex_decoding_table[256];
unsigned char base16hex_decoding_table[256];
unsigned char z85_decoding_table[256];

void init_decoding_table(void) {
  memset(decoding_table, 0x80, 256);
  for (int i = 0; i < 64; ++i) {
    decoding_table[(unsigned char)encoding_table[i]] = i;
  }
}

void init_url_decoding_table(void) {
  memset(url_decoding_table, 0x80, 256);
  for (int i = 0; i < 64; ++i) {
    url_decoding_table[(unsigned char)url_encoding_table[i]] = i;
  }
}

void init_base58_decoding_table(void) {
  memset(base58_decoding_table, 0x80, 256);
  for (int i = 0; i < 58; i++) {
    base58_decoding_table[(unsigned char)base58_alphabet[i]] = i;
  }
}

void init_base32_decoding_table(void) {
  memset(base32_decoding_table, 0x80, 256);
  for (int i = 0; i < 32; i++) {
    base32_decoding_table[(unsigned char)base32_alphabet[i]] = i;
  }
}

void init_base32hex_decoding_table(void) {
  memset(base32hex_decoding_table, 0x80, 256);
  for (int i = 0; i < 32; i++) {
    base32hex_decoding_table[(unsigned char)base32hex_alphabet[i]] = i;
  }
}

void init_base16_decoding_table(void) {
  memset(base16hex_decoding_table, 0x80, 256);
  for (int i = 0; i < 16; i++) {
    base16_decoding_table[(unsigned char)base16_alphabet[i]] = i;
  }

  for (char c = 'a'; c <= 'f'; c++) {
    base16_decoding_table[(unsigned char)c] = 10 + (c - 'a');
  }
}

void init_z85_decoding_table(void) {
  memset(z85_decoding_table, 0xFF, 256);
  for (int i = 0; i < 85; i++) {
    z85_decoding_table[(unsigned char)z85_alphabet[i]] = i;
  }
}

void init_decode_table_wrapper(unsigned int base) {
  if (base == B_64)
    init_decoding_table();
  else if (base == B_64URL)
    init_url_decoding_table();
  else if (base == B_58)
    init_base58_decoding_table();
  else if (base == B_32)
    init_base32_decoding_table();
  else if (base == B_32HEX)
    init_base32hex_decoding_table();
  else if (base == B_16)
    init_base16_decoding_table();
  else if (base == B_Z85)
    init_z85_decoding_table();
}

unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {
  if (input_length % 4 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length / 4 * 3;

  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  int i, j;
  for (i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a =
        data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_b =
        data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_c =
        data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_d =
        data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];

    uint32_t triple = (sextet_a << 3 * 6) | (sextet_b << 2 * 6) |
                      (sextet_c << 1 * 6) | (sextet_d << 0 * 6);

    if (j < *output_length)
      decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

unsigned char *base64url_decode(const char *data, size_t input_length,
                                size_t *output_length) {
  size_t pad = (4 - (input_length % 4)) % 4;

  char *padded = malloc(input_length + pad);
  if (!padded) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  memcpy(padded, data, input_length);
  for (size_t i = 0; i < pad; ++i)
    padded[input_length + i] = '=';

  size_t padded_len = input_length + pad;

  *output_length = padded_len / 4 * 3;
  if (padded[padded_len - 1] == '=')
    (*output_length)--;
  if (padded[padded_len - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    free(padded);
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  int i = 0, j = 0;
  while (i < padded_len) {
    uint32_t a =
        padded[i] == '=' ? 0 : url_decoding_table[(unsigned char)padded[i]];
    i++;
    uint32_t b =
        padded[i] == '=' ? 0 : url_decoding_table[(unsigned char)padded[i]];
    i++;
    uint32_t c =
        padded[i] == '=' ? 0 : url_decoding_table[(unsigned char)padded[i]];
    i++;
    uint32_t d =
        padded[i] == '=' ? 0 : url_decoding_table[(unsigned char)padded[i]];
    i++;

    uint32_t triple = (a << 18) | (b << 12) | (c << 6) | d;

    if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
    if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
    if (j < *output_length) decoded_data[j++] = (triple >> 0) & 0xFF;
  }

  free(padded);
  return decoded_data;
}

unsigned char* base58_decode(const char *data, size_t input_length, size_t *output_length) {
  size_t zeros = 0;
  while (zeros < input_length && data[zeros] == '1') {
    zeros++;
  }

  size_t size = input_length * 733 / 1000 + 1;
  unsigned char *buf = malloc(size);
  if (!buf) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }
  memset(buf, 0, size);

  size_t high = 0;
  for (size_t i = zeros; i < input_length; i++) {
    unsigned char val = base58_decoding_table[(unsigned char)data[i]];
    if (val & 0x80) {
      free(buf);
      return NULL;
    }

    int carry = val;
    size_t j = 0;

    for (ssize_t k = size - 1; (carry != 0 || j < high) && k >= 0; k--, j++) {
      carry += 58 * buf[k];
      buf[k] = carry % 256;
      carry /= 256;
    }
    high = j;
  }

  size_t p = 0;
  while (p < size && buf[p] == 0) p++;

  *output_length = zeros + (size - p);
  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    free(buf); fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno)); return NULL;
  }

  size_t idx = 0;
  for (size_t i = 0; i < zeros; i++) decoded_data[idx++] = 0;
  for (size_t i = p; i < size; i++) decoded_data[idx++] = buf[i];

  free(buf);
  return decoded_data;
}

unsigned char *base32_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 8 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  size_t final_len = input_length / 8 * 5;

  if (input_length > 0 && data[input_length - 1] == '=') final_len--;
  if (input_length > 1 && data[input_length - 2] == '=') final_len--;
  if (input_length > 3 && data[input_length - 3] == '=') final_len--;
  if (input_length > 4 && data[input_length - 4] == '=') final_len--;

  unsigned char *decoded_data = malloc(final_len);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i = 0, j = 0;

  while (i < input_length) {
    uint64_t buffer = 0;
    int valid_bits = 40;

    for (int k = 0; k < 8; k++) {
      char c = data[i++];

      if (c == '=') {
        buffer <<= 5;
        valid_bits -= 5;
      } else {
        unsigned char v = base32_alphabet[(unsigned char)c];
        if (v & 0x80) {
          free(decoded_data);
          return NULL;
        }
        buffer = (buffer << 5) | v;
      }
    }

    int bytes = valid_bits / 8;
    for (int b = 0; b < bytes; b++) {
      decoded_data[j++] = (buffer >> (32 - b * 8)) & 0xFF;
    }
  }

  *output_length = j;
  return decoded_data;
}

unsigned char *base32hex_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 8 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  size_t final_len = input_length / 8 * 5;

  if (input_length > 0 && data[input_length - 1] == '=') final_len--;
  if (input_length > 1 && data[input_length - 2] == '=') final_len--;
  if (input_length > 3 && data[input_length - 3] == '=') final_len--;
  if (input_length > 4 && data[input_length - 4] == '=') final_len--;

  unsigned char *decoded_data = malloc(final_len);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i = 0, j = 0;

  while (i < input_length) {
    uint64_t buffer = 0;
    int valid_bits = 40;

    for (int k = 0; k < 8; k++) {
      char c = data[i++];

      if (c == '=') {
        buffer <<= 5;
        valid_bits -= 5;
      } else {
        unsigned char v = base32hex_alphabet[(unsigned char)c];
        if (v & 0x80) {
          free(decoded_data);
          return NULL;
        }
        buffer = (buffer << 5) | v;
      }
    }

    int bytes = valid_bits / 8;
    for (int b = 0; b < bytes; b++) {
      decoded_data[j++] = (buffer >> (32 - b * 8)) & 0xFF;
    }
  }

  *output_length = j;
  return decoded_data;
}

unsigned char *base16_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 2 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length / 2;
  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  for (size_t i = 0, j = 0; i < input_length; i += 2, j++) {
    unsigned char hi = base16_decoding_table[(unsigned char)data[i]];
    unsigned char lo = base16_decoding_table[(unsigned char)data[i + 1]];

    if ((hi | lo) & 0x80) {
      free(decoded_data);
      fputs("basenc: invalid input\n", stderr);
      return NULL;
    }

    decoded_data[j] = (hi << 4) | lo;
  }

  return decoded_data;
}

unsigned char *base2msbf_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 8 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length / 8;
  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;
  for (size_t i = 0; i < input_length; i += 8) {
    unsigned char b = 0;
    for (int bit = 0; bit < 8; bit++) {
      char c = data[i + bit];
      if (c != '0' && c != '1') {
        free(decoded_data);
        return NULL;
      }
      b = (b << 1) | (unsigned char)(c - '0');
    }
    decoded_data[j++] = b;
  }

  return decoded_data;
}

unsigned char *base2lsbf_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 8 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length / 8;
  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;
  for (size_t i = 0; i < input_length; i += 8) {
    unsigned char b = 0;
    for (int bit = 0; bit < 8; bit++) {
      char c = data[i + bit];
      if (c != '0' && c != '1') {
        fputs("basenc: invalid input\n", stderr);
        free(decoded_data);
        return NULL;
      }
      b |= ((unsigned char)(c - '0') << bit);
    }
    decoded_data[j++] = b;
  }

  return decoded_data;
}

unsigned char* z85_decode(const char *data, size_t input_length, size_t *output_length) {
  if (input_length % 5 != 0) {
    fputs("basenc: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length * 4 / 5;
  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "basenc: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;

  for (size_t i = 0; i < input_length; i += 5) {
    uint32_t value = 0;

    for (int k = 0; k < 5; k++) {
      unsigned char v = z85_decoding_table[(unsigned char)data[i + k]];
      if (v == 0xFF) {
        free(decoded_data);
        return NULL;
      }
      value = value * 85 + v;
    }

    decoded_data[j + 0] = (value >> 24) & 0xFF;
    decoded_data[j + 1] = (value >> 16) & 0xFF;
    decoded_data[j + 2] = (value >> 8) & 0xFF;
    decoded_data[j + 3] = value & 0xFF;
    j += 4;
  }

  return decoded_data;
}
