#pragma once

#include <string>

template <typename I>
std::string tohex(I w, size_t hex_len = 4) {
  static const char* digits = "0123456789ABCDEF";
  std::string rc(hex_len, '0');
  for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4) {
    rc[i] = digits[(w >> j) & 0x0f];
  }
  return "0x" + rc;
}