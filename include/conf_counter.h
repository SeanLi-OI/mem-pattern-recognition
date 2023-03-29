//Author: Lixiang

#ifndef CONF_COUNTER_H
#define CONF_COUNTER_H

class ConfCounter {
 private:
  int threshold_lower;
  int threshold_upper;
  int counter;

 public:
  ConfCounter() {
    counter = 8;
    threshold_lower = 3;
    threshold_upper = 256;
  }
  bool Positive() {
    if (counter >= threshold_upper) return true;
    counter++;
    return false;
  }
  bool Negative() {
    if (counter <= threshold_lower) return true;
    counter >>= 1;
    return false;
  }
  bool Dec() {
    if (counter <= threshold_lower) return true;
    counter--;
    return false;
  }
  bool test() { return counter > threshold_lower; }
  void reset() { counter = 8; }
  int get() { return counter; }
};

#endif