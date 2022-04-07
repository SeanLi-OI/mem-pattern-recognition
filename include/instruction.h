#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "circular_buffer.hpp"

// instruction format
#define NUM_INSTR_DESTINATIONS_SPARC 4
#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

// special registers that help us identify branches
#define REG_STACK_POINTER 6
#define REG_FLAGS 25
#define REG_INSTRUCTION_POINTER 26

// branch types
#define NOT_BRANCH 0
#define BRANCH_DIRECT_JUMP 1
#define BRANCH_INDIRECT 2
#define BRANCH_CONDITIONAL 3
#define BRANCH_DIRECT_CALL 4
#define BRANCH_INDIRECT_CALL 5
#define BRANCH_RETURN 6
#define BRANCH_OTHER 7

class LSQ_ENTRY;

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
