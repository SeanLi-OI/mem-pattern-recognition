#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

struct MemRecord {
  unsigned long long int addr;
  uint8_t content[8];
  unsigned len;
};

struct MsRecord {
  unsigned long long int ip;
  unsigned tid;

  MemRecord r0;
  MemRecord r1;
  MemRecord w0;
};

struct MyInstr {
  unsigned long long int ip;
  unsigned tid;
  std::string func_name;
  std::string image;

  MemRecord r0;
  MemRecord r1;
  MemRecord w0;
  MyInstr() {}
  MyInstr(const MsRecord &a) {
    ip = a.ip;
    tid = a.tid;
    r0 = a.r0;
    r1 = a.r1;
    w0 = a.w0;
    func_name = "";
    image = "";
  }
};

#endif
