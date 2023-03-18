//Author: Lixiang

#include "utils/macro.h"

bool trace_filter(const unsigned long long int &ip, const bool &isWrite,
                  const unsigned long long int &value) {
  if (!isWrite && value == ip) return true;
  return false;
}