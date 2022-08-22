#ifndef MACRO_H
#define MARCO_H

// #define ENABLE_PC_DIFF

#include <iomanip>
#include <iostream>
#include <string>

#define RAW_PERCENT(CNT, TOTAL)               \
  std::setprecision(4) << (((TOTAL) == 0) ? 0 \
                                          : ((CNT) * (double)100.0 / (TOTAL)))

#define PERCENT(CNT, TOTAL)     \
  "  (" << std::setprecision(4) \
        << (((TOTAL) == 0) ? 0 : ((CNT) * (double)100.0 / (TOTAL))) << "%)"

#define PERCENT_WITH_OP(CNT, TOTAL)                      \
  "  (" << (CNT >= 0 ? "+" : "") << std::setprecision(4) \
        << (((TOTAL) == 0) ? 0 : ((CNT) * (double)100.0 / (TOTAL))) << "%)"

#define MY_ALIGN(n) std::left << std::setw(12) << n

#define MY_ALIGN_W(n, w) std::left << std::setw(w) << n

#define MY_ALIGN_STR(s) std::left << std::setw(15) << std::setfill(' ') << s

#define DIFF(n1, n2)                                    \
  (std::string("(") + std::string(n1 > n2 ? "+" : "") + \
   std::to_string((long long)n1 - (long long)n2) + std::string(")"))

#define DIFF_FLOAT(n1, n2)                              \
  (std::string("(") + std::string(n1 > n2 ? "+" : "") + \
   std::to_string((double)n1 - (double)n2) + std::string(")"))

bool trace_filter(const unsigned long long int &ip, const bool &isWrite,
                  const unsigned long long int &value);

#endif