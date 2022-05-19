
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "pin.H"

using std::map;
using std::pair;
using std::string;
using std::vector;

const static size_t REG_SIZE = 8;

#define TRACE_FUNC_NAME
// #define DEBUG
struct MemRecord {
  ADDRINT addr;
  uint8_t content[REG_SIZE];
  uint32_t len;
};

struct MsRecord {
  ADDRINT ip;
  THREADID tid;

  MemRecord r0;
  MemRecord r1;
  MemRecord w0;
};

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

typedef struct trace_instr_format {
  unsigned long long int ip;  // instruction pointer (program counter) value

  unsigned char is_branch;     // is this branch
  unsigned char branch_taken;  // if so, is this taken

  unsigned char
      destination_registers[NUM_INSTR_DESTINATIONS];  // output registers
  unsigned char source_registers[NUM_INSTR_SOURCES];  // input registers

  unsigned long long int
      destination_memory[NUM_INSTR_DESTINATIONS];           // output memory
  unsigned long long int source_memory[NUM_INSTR_SOURCES];  // input memory
} trace_instr_format_t;

/* ================================================================== */
// Global variables
/* ================================================================== */

UINT64 instrCount = 0;

FILE *out;
FILE *out_mpr;

bool output_file_closed = false;
bool tracing_on = false;
bool trace_this;
string func_name;
string image;

vector<std::pair<unsigned long long int, unsigned long long int> > roi_pc;
unsigned long long int end_ip;

trace_instr_format_t curr_instr;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<std::string> KnobOutputFile(
    KNOB_MODE_WRITEONCE, "pintool", "o", "champsim.trace",
    "specify file name for Champsim tracer output");

KNOB<std::string> KnobMPROutputFile(KNOB_MODE_WRITEONCE, "pintool", "m",
                                    "mpr.trace",
                                    "specify file name for MPR tracer output");

KNOB<UINT64> KnobSkipInstructions(
    KNOB_MODE_WRITEONCE, "pintool", "s", "0",
    "How many instructions to skip before tracing begins");

KNOB<UINT64> KnobTraceInstructions(KNOB_MODE_WRITEONCE, "pintool", "t",
                                   "3000000000",
                                   "How many instructions to trace");

KNOB<std::string> KnobROIFile(KNOB_MODE_WRITEONCE, "pintool", "r", "roi.txt",
                              "specify file name for ROI file");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage() {
  std::cerr
      << "This tool creates a register and memory access trace" << std::endl
      << "Specify the champsim output trace file with -o" << std::endl
      << "Specify the mpr output trace file with -m" << std::endl
      << "Specify the ROI input file with -r" << std::endl
      << "Specify the number of instructions to skip before tracing with -s"
      << std::endl
      << "Specify the number of instructions to trace with -t" << std::endl
      << std::endl;

  std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;

  return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

void BeginInstruction(VOID *ip, UINT32 op_code, VOID *opstring) {
  if (tracing_on) {
    // if ((unsigned long long int)ip > end_ip) tracing_on = false;
    bool flag = true;
    for (auto &kv : roi_pc) {
      if ((unsigned long long int)ip >= kv.first &&
          (unsigned long long int)ip <= kv.second) {
        flag = false;
        break;
      }
    }
    if (flag) tracing_on = false;
  }
  if (!tracing_on) {
    for (auto &kv : roi_pc) {
      if ((unsigned long long int)ip >= kv.first &&
          (unsigned long long int)ip <= kv.second) {
        // end_ip = kv.second;
        tracing_on = true;
        break;
      }
    }
  }
  if (!tracing_on) return;

  instrCount++;
  if (instrCount <= KnobSkipInstructions.Value()) tracing_on = false;
  // printf("%llx\n",(unsigned long long int)ip);
  // printf("[%p %u %s ", ip, op_code, (char*)opstring);

  //   if (instrCount > KnobSkipInstructions.Value()) {
  //     tracing_on = true;

  //     if (instrCount >
  //         (KnobTraceInstructions.Value() + KnobSkipInstructions.Value()))
  //       tracing_on = false;
  //   }

  // reset the current instruction
  curr_instr.ip = (unsigned long long int)ip;

  curr_instr.is_branch = 0;
  curr_instr.branch_taken = 0;

  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    curr_instr.destination_registers[i] = 0;
    curr_instr.destination_memory[i] = 0;
  }

  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    curr_instr.source_registers[i] = 0;
    curr_instr.source_memory[i] = 0;
  }
  trace_this = false;
}

void EndInstruction() {
  // printf("%d]\n", (int)instrCount);

  // printf("\n");

  if (instrCount > KnobSkipInstructions.Value()) {
    tracing_on = true;

    if (instrCount <=
        (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
      // keep tracing
      fwrite(&curr_instr, sizeof(trace_instr_format_t), 1, out);
    } else {
      tracing_on = false;
      // close down the file, we're done tracing
      if (!output_file_closed) {
        fclose(out);
        output_file_closed = true;
      }

      exit(0);
    }
  }
}

void BranchOrNot(UINT32 taken) {
  // printf("[%d] ", taken);

  curr_instr.is_branch = 1;
  if (taken != 0) {
    curr_instr.branch_taken = 1;
  }
}

void RegRead(UINT32 i, UINT32 index) {
  if (!tracing_on) return;

  REG r = (REG)i;

  /*
     if(r == 26)
     {
  // 26 is the IP, which is read and written by branches
  return;
  }
  */

  // cout << r << " " << REG_StringShort((REG)r) << " " ;
  // cout << REG_StringShort((REG)r) << " " ;

  // printf("%d ", (int)r);

  // check to see if this register is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (curr_instr.source_registers[i] == ((unsigned char)r)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (curr_instr.source_registers[i] == 0) {
        curr_instr.source_registers[i] = (unsigned char)r;
        break;
      }
    }
  }
}

void RegWrite(REG i, UINT32 index) {
  if (!tracing_on) return;

  REG r = (REG)i;

  /*
     if(r == 26)
     {
  // 26 is the IP, which is read and written by branches
  return;
  }
  */

  // cout << "<" << r << " " << REG_StringShort((REG)r) << "> ";
  // cout << "<" << REG_StringShort((REG)r) << "> ";

  // printf("<%d> ", (int)r);

  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (curr_instr.destination_registers[i] == ((unsigned char)r)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (curr_instr.destination_registers[i] == 0) {
        curr_instr.destination_registers[i] = (unsigned char)r;
        break;
      }
    }
  }
  /*
     if(index==0)
     {
     curr_instr.destination_register = (unsigned long long int)r;
     }
     */
}

void MemoryRead(VOID *addr, UINT32 index, UINT32 read_size) {
  if (!tracing_on) return;

  // printf("0x%llx,%u ", (unsigned long long int)addr, read_size);

  // check to see if this memory read location is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (curr_instr.source_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (curr_instr.source_memory[i] == 0) {
        curr_instr.source_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
}

void MemoryWrite(VOID *addr, UINT32 index) {
  if (!tracing_on) return;

  // printf("(0x%llx) ", (unsigned long long int) addr);

  // check to see if this memory write location is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (curr_instr.destination_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (curr_instr.destination_memory[i] == 0) {
        curr_instr.destination_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
  /*
     if(index==0)
     {
     curr_instr.destination_memory = (long long int)addr;
     }
     */
}

#ifdef DEBUG
static void print(ADDRINT ip, MemRecord &r, bool isWrite) {
  if (r.len == 0) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  fprintf(stderr, "%c %llu %llu %llu\n", isWrite ? 'W' : 'R',
          (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}
#endif

static void fwritestr(string str, FILE *f) {
  size_t len = str.length();
  fwrite(&len, sizeof(len), 1, f);
  fwrite(&str[0], len, 1, f);
}

static void rec_inst(ADDRINT ip, MsRecord &k) {
  k.ip = ip;
  k.tid = PIN_ThreadId();
  fwrite(&k, sizeof(MsRecord), 1, out_mpr);
#ifdef TRACE_FUNC_NAME
  fwritestr(func_name, out_mpr);
  fwritestr(image, out_mpr);
#endif
  trace_this = true;
#ifdef DEBUG
  print(ip, k.r0, 0);
  print(ip, k.r1, 0);
  print(ip, k.w0, 1);
#endif
}

static VOID recorder_rrw(ADDRINT ip, VOID *addr0, UINT32 len0, VOID *addr1,
                         UINT32 len1, VOID *addr2, UINT32 len2) {
  if (!tracing_on) return;

  // if (len0 > REG_SIZE || len1 > REG_SIZE ||
  //     len2 > REG_SIZE)  // ignore this one!
  //   return;
  if (len0 > REG_SIZE) len0 = 0;
  if (len1 > REG_SIZE) len1 = 0;
  if (len2 > REG_SIZE) len2 = 0;

  static MsRecord k;

  k.r0.addr = reinterpret_cast<ADDRINT>(addr0);
  k.r1.addr = reinterpret_cast<ADDRINT>(addr1);
  k.w0.addr = reinterpret_cast<ADDRINT>(addr2);
  k.r0.len = len0;
  k.r1.len = len1;
  k.w0.len = len2;

  PIN_SafeCopy(k.r0.content, static_cast<UINT8 *>(addr0), len0);
  PIN_SafeCopy(k.r1.content, static_cast<UINT8 *>(addr1), len1);
  PIN_SafeCopy(k.w0.content, static_cast<UINT8 *>(addr2), len2);

  rec_inst(ip, k);
}

static VOID recorder_rw(ADDRINT ip, VOID *addr0, UINT32 len0, VOID *addr2,
                        UINT32 len2) {
  if (!tracing_on) return;

  // if (len0 > REG_SIZE || len2 > REG_SIZE)  // ignore this one!
  //   return;

  if (len0 > REG_SIZE) len0 = 0;
  if (len2 > REG_SIZE) len2 = 0;

  static MsRecord k;
  k.r1.len = 0;

  k.r0.addr = reinterpret_cast<ADDRINT>(addr0);
  k.w0.addr = reinterpret_cast<ADDRINT>(addr2);
  k.r0.len = len0;
  k.w0.len = len2;

  PIN_SafeCopy(k.r0.content, static_cast<UINT8 *>(addr0), len0);
  PIN_SafeCopy(k.w0.content, static_cast<UINT8 *>(addr2), len2);

  rec_inst(ip, k);
}

static VOID recorder_rr(ADDRINT ip, VOID *addr0, UINT32 len0, VOID *addr1,
                        UINT32 len1) {
  if (!tracing_on) return;

  // if (len0 > REG_SIZE || len1 > REG_SIZE)  // ignore this one!
  //   return;

  if (len0 > REG_SIZE) len0 = 0;
  if (len1 > REG_SIZE) len1 = 0;

  static MsRecord k;
  k.w0.len = 0;

  k.r0.addr = reinterpret_cast<ADDRINT>(addr0);
  k.r1.addr = reinterpret_cast<ADDRINT>(addr1);
  k.r0.len = len0;
  k.r1.len = len1;

  PIN_SafeCopy(k.r0.content, static_cast<UINT8 *>(addr0), len0);
  PIN_SafeCopy(k.r1.content, static_cast<UINT8 *>(addr1), len1);

  rec_inst(ip, k);
}

static VOID recorder_r(ADDRINT ip, VOID *addr0, UINT32 len0) {
  if (!tracing_on) return;

  // if (len0 > REG_SIZE)  // ignore this one!
  //   return;
  if (len0 > REG_SIZE) len0 = 0;

  static MsRecord k;
  k.r1.len = k.w0.len = 0;

  k.r0.addr = reinterpret_cast<ADDRINT>(addr0);
  k.r0.len = len0;

  PIN_SafeCopy(k.r0.content, static_cast<UINT8 *>(addr0), len0);

  rec_inst(ip, k);
}

static VOID recorder_w(ADDRINT ip, VOID *addr2, UINT32 len2) {
  if (!tracing_on) return;

  // if (len2 > REG_SIZE)  // ignore this one!
  //   return;
  if (len2 > REG_SIZE) len2 = 0;

  static MsRecord k;
  k.r0.len = k.r1.len = 0;

  k.w0.addr = reinterpret_cast<ADDRINT>(addr2);
  k.w0.len = len2;

  PIN_SafeCopy(k.w0.content, static_cast<UINT8 *>(addr2), len2);

  rec_inst(ip, k);
}

const char *StripPath(const char *path) {
  const char *file = strrchr(path, '/');
  if (file)
    return file + 1;
  else
    return path;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {
  // begin each instruction with this function
  UINT32 opcode = INS_Opcode(ins);
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BeginInstruction, IARG_INST_PTR,
                 IARG_UINT32, opcode, IARG_END);

  // instrument branch instructions
  if (INS_IsBranch(ins))
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchOrNot, IARG_BRANCH_TAKEN,
                   IARG_END);

  // instrument register reads
  UINT32 readRegCount = INS_MaxNumRRegs(ins);
  for (UINT32 i = 0; i < readRegCount; i++) {
    UINT32 regNum = INS_RegR(ins, i);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RegRead, IARG_UINT32, regNum,
                   IARG_UINT32, i, IARG_END);
  }

  // instrument register writes
  UINT32 writeRegCount = INS_MaxNumWRegs(ins);
  for (UINT32 i = 0; i < writeRegCount; i++) {
    UINT32 regNum = INS_RegW(ins, i);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RegWrite, IARG_UINT32, regNum,
                   IARG_UINT32, i, IARG_END);
  }

  // instrument memory reads and writes
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      UINT32 read_size = INS_MemoryOperandSize(ins, memOp);

      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemoryRead, IARG_MEMORYOP_EA,
                     memOp, IARG_UINT32, memOp, IARG_UINT32, read_size,
                     IARG_END);
    }
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemoryWrite, IARG_MEMORYOP_EA,
                     memOp, IARG_UINT32, memOp, IARG_END);
    }
  }
  if (RTN_Valid(INS_Rtn(ins))) {
    func_name = RTN_Name(INS_Rtn(ins));
    image = StripPath(IMG_Name(SEC_Img(RTN_Sec(INS_Rtn(ins)))).c_str());
  }

  if (INS_IsMemoryRead(ins) && INS_HasMemoryRead2(ins) &&
      INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)recorder_rrw, IARG_INST_PTR,
                   IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
                   IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE,
                   IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
  } else if (INS_IsMemoryRead(ins) && !INS_HasMemoryRead2(ins) &&
             INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)recorder_rw, IARG_INST_PTR,
                   IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
                   IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
  } else if (INS_IsMemoryRead(ins) && INS_HasMemoryRead2(ins) &&
             !INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)recorder_rr, IARG_INST_PTR,
                   IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
                   IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_END);
  } else if (!INS_IsMemoryRead(ins) && !INS_HasMemoryRead2(ins) &&
             INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)recorder_w, IARG_INST_PTR,
                   IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
  } else if (INS_IsMemoryRead(ins) && !INS_HasMemoryRead2(ins) &&
             !INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)recorder_r, IARG_INST_PTR,
                   IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
  }

  // finalize each instruction with this function
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)EndInstruction, IARG_END);
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v) {
  // close the file if it hasn't already been closed
  if (!output_file_closed) {
    fclose(out);
    output_file_closed = true;
  }
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet
 * started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[]) {
  // Initialize PIN library. Print help message if -h(elp) is specified
  // in the command line or the command line is invalid
  if (PIN_Init(argc, argv)) return Usage();

  const char *fileName = KnobOutputFile.Value().c_str();

  out = fopen(fileName, "ab");
  if (!out) {
    std::cout << "Couldn't open output trace file. Exiting." << std::endl;
    exit(1);
  }

  const char *fileName2 = KnobMPROutputFile.Value().c_str();

  out_mpr = fopen(fileName2, "ab");
  if (!out_mpr) {
    std::cout << "Couldn't open MPR output trace file. Exiting." << std::endl;
    exit(1);
  }

  FILE *in = fopen(KnobROIFile.Value().c_str(), "r");
  if (!in) {
    std::cout << "Couldn't open ROI input file. Exiting." << std::endl;
    exit(1);
  }
  unsigned long long int pc1, pc2;

  while (fscanf(in, "%llx %llx", &pc1, &pc2) != EOF) {
    roi_pc.emplace_back(std::make_pair(pc1, pc2));
    std::cerr << pc1 << " " << pc2 << std::endl;
  }

  // Register function to be called to instrument instructions
  INS_AddInstrumentFunction(Instruction, 0);

  // Register function to be called when the application exits
  PIN_AddFiniFunction(Fini, 0);

  std::cerr << "===============================================" << std::endl;
  std::cerr << "This application is instrumented by the MPR Trace Generator"
            << std::endl;
  std::cerr << "Read ROI point from " << KnobROIFile.Value().c_str()
            << std::endl;
  std::cerr << "Trace saved in " << fileName << std::endl;
  std::cerr << "           and " << fileName2 << std::endl;
  std::cerr << "Skip inst: " << KnobSkipInstructions << std::endl;
  std::cerr << "Run inst:" << KnobTraceInstructions << std::endl;
  std::cerr << "===============================================" << std::endl;
  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
