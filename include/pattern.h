#ifndef PATTERN_H
#define PATTERN_H

#include <string>

const int PATTERN_NUM = 10;
const std::string PATTERN_NAME[] = {
    "fresh",   "struct_pointer", "static",        "stride",
    "pointer", "pointer_chase",  "pointer_array", "indirect",
    "random",   "other"};  // Make sure PATTERN_NAME has same order with
                          // PATTERN
enum PATTERN : uint16_t {
  FRESH,
  STRUCT_POINTER,
  STATIC,
  STRIDE,
  pointer,
  POINTER_A,
  POINTER_B,
  INDIRECT,
  RANDOM,
  OTHER  // Make sure other is the last one
};

static std::unordered_map<std::string, PATTERN> const pattern_table = {
    {"fresh", PATTERN::FRESH},
    {"struct_pointer", PATTERN::STRUCT_POINTER},
    {"static", PATTERN::STATIC},
    {"stride", PATTERN::STRIDE},
    {"pointer", PATTERN::pointer},
    {"pointer_chase", PATTERN::POINTER_A},
    {"pointer_array", PATTERN::POINTER_B},
    {"indirect", PATTERN::INDIRECT},
    {"random", PATTERN::RANDOM},
    {"other", PATTERN::OTHER}};

template <typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

const uint32_t INTERVAL = 256;
const uint16_t STRIDE_THERSHOLD = 512;
const uint16_t POINTER_A_THERSHOLD = 512;
const uint16_t INDIRECT_THERSHOLD = 512;
const uint16_t PATTERN_THERSHOLD = 512;
const uint16_t POINTER_THERSHOLD = 512;
const uint16_t STATIC_THERSHOLD = 512;
const uint16_t STRUCT_POINTER_THERSHOLD = 512;
const uint16_t RANDOM_THERSHOLD = 512;
const uint16_t RANDOM_LEN = 64;
const uint32_t RANDOM_T = 256;

#endif