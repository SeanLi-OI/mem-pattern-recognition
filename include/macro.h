#ifndef MACRO_H
#define MARCO_H

// #define ENABLE_PC_DIFF

#include <iomanip>
#include <iostream>

#define PERCENT(CNT, TOTAL)     \
  "  (" << std::setprecision(4) \
        << (((TOTAL) == 0) ? 0 : ((CNT) * (double)100.0 / (TOTAL))) << "%)"

#define MY_ALIGN(n) std::left << std::setw(12) << (n)

#endif