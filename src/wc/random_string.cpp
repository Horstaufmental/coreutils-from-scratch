#include <cstddef>
#include <fstream>
#include <random>
#include <string>

std::string to_utf8(char32_t codepoint) {
  std::string utf8_char;
  if (codepoint < 0x80) {
    utf8_char += static_cast<char>(codepoint);
  } else if (codepoint < 0x800) {
    utf8_char += static_cast<char>(0xC0 | (codepoint >> 6));
    utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
  } else if (codepoint < 0x10000) {
    utf8_char += static_cast<char>(0xE0 | (codepoint >> 12));
    utf8_char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
  } else if (codepoint < 0x110000) {
    utf8_char += static_cast<char>(0xF0 | (codepoint >> 18));
    utf8_char += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
    utf8_char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
  }
  return utf8_char;
}

std::string generate_random_utf8_string(int length) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<char32_t> distrib(
      0x0020, 0x10FFFF); // Example range: printable ASCII to max Unicode

  std::string random_string;
  for (int i = 0; i < length; ++i) {
    char32_t codepoint = distrib(gen);
    if ((codepoint >= 0xD800 && codepoint <= 0xDFFF) || // Surrogates
        (codepoint >= 0xFDD0 && codepoint <= 0xFDEF) || // Noncharacters
        (codepoint >= 0xFFFE && codepoint <= 0xFFFF)) { // Noncharacters
      // Skip invalid or non-printable code points, or re-generate
      --i; // Decrement i to generate another character for this position
      continue;
    }
    random_string += to_utf8(codepoint);
  }
  return random_string;
}

std::string generate_random_str(size_t length) {
  const std::string chars =
      "abcdefghijklmnopqrstuvwxyz1234567890@#$_&-+()/*':;!?~`^={}\\\"%[]\n\t";
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<size_t> distribution(0, chars.length() - 1);

  std::string random_str;
  random_str.reserve(length);

  for (size_t i = 0; i < length; i++) {
    random_str += chars[distribution(generator)];
  }

  return random_str;
}

int main(void) {
  std::ofstream output("gibberish.txt");
  std::ofstream output_utf8("gibberish-utf8-ascii-mix.txt");

  if (output.is_open()) {
    output << generate_random_str(150000000) << std::endl;
    output.close();
  } else {
    return 1;
  }

  if (output_utf8.is_open()) {
    output_utf8 << generate_random_str(75000000) << generate_random_utf8_string(75000000) << std::endl;
    output_utf8.close();
  } else {
    return 1;
  }
  return 0;
}
