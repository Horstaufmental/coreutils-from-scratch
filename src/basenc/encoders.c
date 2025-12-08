#include "encoders.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

const char base64_encoding_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64url_encoding_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
const char base58_alphabet[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
const char base32_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
const char base32hex_alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
const char base16_alphabet[] = "0123456789ABCDEF";
const char z85_alphabet[] = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            ".-:+=^!/*?&<>()[]{}@%$#";

char *base64_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  *output_length = 4 * ((input_length + 2) / 3);
  char *encoded_data = (char *)malloc(*output_length + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i, j;
  for (i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  int mod = input_length % 3;
  if (mod == 1) {
    encoded_data[*output_length - 2] = '=';
    encoded_data[*output_length - 1] = '=';
  } else if (mod == 2) {
    encoded_data[*output_length - 1] = '=';
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}


char *base64url_encode(const unsigned char *data, size_t input_length,
                       size_t *output_length) {
  size_t full_len = 4 * ((input_length + 2) / 3);
  char *encoded_data = malloc(full_len + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i = 0, j = 0;
  while (i < input_length) {
    uint32_t a = i < input_length ? data[i++] : 0;
    uint32_t b = i < input_length ? data[i++] : 0;
    uint32_t c = i < input_length ? data[i++] : 0;

    uint32_t triple = (a << 16) | (b << 8) | c;

    encoded_data[j++] = base64url_encoding_table[(triple >> 18) & 0x3F];
    encoded_data[j++] = base64url_encoding_table[(triple >> 12) & 0x3F];
    encoded_data[j++] = base64url_encoding_table[(triple >> 6) & 0x3F];
    encoded_data[j++] = base64url_encoding_table[(triple >> 0) & 0x3F];
  }

  int mod = input_length % 3;
  if (mod == 1)
    j -= 2;
  else if (mod == 2)
    j -= 1;

  encoded_data[j] = '\0';
  *output_length = j;

  return encoded_data;
}

char *base58_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  size_t zeros = 0;
  while (zeros < input_length && data[zeros] == 0) {
    zeros++;
  }

  size_t size = input_length * 138 / 100 + 1;
  unsigned char *buf = malloc(size);
  if (!buf) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }
  memset(buf, 0, size);

  size_t high = 0;
  for (size_t i = zeros; i < input_length; i++) {
    int carry = data[i];
    size_t j = 0;

    for (ssize_t k = size - 1; (carry != 0 || j < high) && k >= 0; k--, j++) {
      carry += 256 * buf[k];
      buf[k] = carry % 58;
      carry /= 58;
    }
    high = j;
  }

  size_t p = 0;
  while (p < size && buf[p] == 0)
    p++;

  *output_length = zeros + (size - p);
  char *encoded_data = malloc(*output_length + 1);
  if (!encoded_data) {
    free(buf);
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t idx = 0;
  for (size_t i = 0; i < zeros; i++)
    encoded_data[idx++] = '1';

  for (size_t i = p; i < size; i++) {
    encoded_data[idx++] = base58_alphabet[buf[i]];
  }

  encoded_data[idx] = '\0';
  free(buf);
  return encoded_data;
}

char *base32_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  size_t out_len = ((input_length + 4) / 5) * 8;
  char *encoded_data = malloc(out_len + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i = 0, j = 0;

  while (i < input_length) {
    uint64_t buffer = 0;
    size_t bytes = input_length - i < 5 ? input_length - i : 5;

    for (size_t k = 0; k < bytes; k++) {
      buffer <<= 8;
      buffer |= data[i++];
    }
    buffer <<= (5 - bytes) * 8;

    encoded_data[j++] = base32_alphabet[(buffer >> 35) & 0x1F];
    encoded_data[j++] = base32_alphabet[(buffer >> 30) & 0x1F];
    encoded_data[j++] = base32_alphabet[(buffer >> 25) & 0x1F];
    encoded_data[j++] = base32_alphabet[(buffer >> 20) & 0x1F];
    encoded_data[j++] =
        (bytes >= 2) ? base32_alphabet[(buffer >> 15) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 3) ? base32_alphabet[(buffer >> 10) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 4) ? base32_alphabet[(buffer >> 5) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 5) ? base32_alphabet[(buffer >> 0) & 0x1F] : '=';
  }

  encoded_data[j] = '\0';
  *output_length = j;
  return encoded_data;
}

char *base32hex_encode(const unsigned char *data, size_t input_length,
                       size_t *output_length) {
  size_t out_len = ((input_length + 4) / 5) * 8;
  char *encoded_data = malloc(out_len + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i = 0, j = 0;

  while (i < input_length) {
    uint64_t buffer = 0;
    size_t bytes = input_length - i < 5 ? input_length - i : 5;

    for (size_t k = 0; k < bytes; k++) {
      buffer <<= 8;
      buffer |= data[i++];
    }
    buffer <<= (5 - bytes) * 8;

    encoded_data[j++] = base32hex_alphabet[(buffer >> 35) & 0x1F];
    encoded_data[j++] = base32hex_alphabet[(buffer >> 30) & 0x1F];
    encoded_data[j++] = base32hex_alphabet[(buffer >> 25) & 0x1F];
    encoded_data[j++] = base32hex_alphabet[(buffer >> 20) & 0x1F];
    encoded_data[j++] =
        (bytes >= 2) ? base32hex_alphabet[(buffer >> 15) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 3) ? base32hex_alphabet[(buffer >> 10) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 4) ? base32hex_alphabet[(buffer >> 5) & 0x1F] : '=';
    encoded_data[j++] =
        (bytes >= 5) ? base32hex_alphabet[(buffer >> 0) & 0x1F] : '=';
  }

  encoded_data[j] = '\0';
  *output_length = j;
  return encoded_data;
}

char *base16_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  *output_length = input_length * 2;
  char *encoded_data = malloc(*output_length + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  for (size_t i = 0, j = 0; i < input_length; i++) {
    encoded_data[j++] = base16_alphabet[(data[i] >> 4) & 0xF];
    encoded_data[j++] = base16_alphabet[data[i] & 0xF];
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}

char *base2msbf_encode(const unsigned char *data, size_t input_length,
                       size_t *output_length) {
  *output_length = input_length * 8;
  char *encoded_data = malloc(*output_length + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;
  for (size_t i = 0; i < input_length; i++) {
    unsigned char b = data[i];
    for (int bit = 7; bit >= 0; bit--) {
      encoded_data[j++] = ((b >> bit) & 1) ? '1' : '0';
    }
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}

char *base2lsbf_encode(const unsigned char *data, size_t input_length,
                       size_t *output_length) {
  *output_length = input_length * 8;
  char *encoded_data = malloc(*output_length + 1);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;
  for (size_t i = 0; i < input_length; i++) {
    unsigned char b = data[i];
    for (int bit = 0; bit < 8; bit++) {
      encoded_data[j++] = ((b >> bit) & 1) ? '1' : '0';
    }
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}

char *z85_encode(const unsigned char *data, size_t input_length,
                 size_t *output_length) {
  if (input_length % 4 != 0) {
    fprintf(stderr, "basenc: invalid input (length must be a multiple of 4 characters)\n");
    return NULL;
  }

  *output_length = input_length * 5 / 4;
  char *encoded_data = malloc(*output_length);
  if (!encoded_data) {
    fprintf(stderr, "basenc: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t j = 0;

  for (size_t i = 0; i < input_length; i += 4) {
    uint32_t value =
        ((uint32_t)data[i + 0] << 24) | ((uint32_t)data[i + 1] << 16) |
        ((uint32_t)data[i + 2] << 8) | ((uint32_t)data[i + 3] << 0);

    char block[5];
    for (int k = 4; k >= 0; k--) {
      block[k] = z85_alphabet[value % 85];
      value /= 85;
    }
    memcpy(encoded_data + j, block, 5);
    j += 5;
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}
