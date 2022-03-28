#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "pin.H"

using std::map;
using std::vector;

const static size_t REG_SIZE = 8;

#define DEBUG
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

/* ================================================================== */
// Global variables
/* ================================================================== */
UINT64 instrCount = 0;
FILE *out;
bool output_file_closed = false;
bool tracing_on = false;

/* ================================================================== */
// instrumentation routines
/* ================================================================== */

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o",
                                 "mpr.trace",
                                 "specify file name for MPR tracer output");

KNOB<UINT64> KnobSkipInstructions(
    KNOB_MODE_WRITEONCE, "pintool", "s", "0",
    "How many instructions to skip before tracing begins");

KNOB<UINT64> KnobTraceInstructions(KNOB_MODE_WRITEONCE, "pintool", "t",
                                   "1000000000",
                                   "How many instructions to trace");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage() {
  std::cerr
      << "This tool creates a register and memory access trace" << std::endl
      << "Specify the output trace file with -o" << std::endl
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
  instrCount++;
  // printf("[%p %u %s ", ip, opcode, (char*)opstring);

  if (instrCount > KnobSkipInstructions.Value()) {
    tracing_on = true;

    if (instrCount >
        (KnobTraceInstructions.Value() + KnobSkipInstructions.Value()))
      tracing_on = false;
  }
  //   fprintf(stderr, "PC: %llu MemOps: %u\n", (unsigned long long)ip,
  //           (unsigned)mem_ops);

  if (!tracing_on) return;
}

void EndInstruction() {
  if (instrCount > KnobSkipInstructions.Value()) {
    tracing_on = true;
    if (instrCount <=
        (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
      // keep tracing
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

#ifdef DEBUG
static void print(ADDRINT ip, MemRecord &r, bool isWrite) {
  if (r.len == 0) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  fprintf(stderr, "%c %llu %llu %llu\n", isWrite ? 'W' : 'R', (unsigned long long)ip,
         (unsigned long long)r.addr, tmp);
}
#endif

static void rec_inst(ADDRINT ip, MsRecord &k) {
  k.ip = ip;
  k.tid = PIN_ThreadId();
  fwrite(&k, sizeof(MsRecord), 1, out);
  #ifdef DEBUG
  print(ip,k.r0,0);
  print(ip,k.r1,0);
  print(ip,k.w0,1);
  #endif
}

static VOID recorder_rrw(ADDRINT ip, VOID *addr0, UINT32 len0, VOID *addr1,
                         UINT32 len1, VOID *addr2, UINT32 len2) {
  if (!tracing_on) return;

  if (len0 > REG_SIZE || len1 > REG_SIZE ||
      len2 > REG_SIZE)  // ignore this one!
    return;

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

  if (len0 > REG_SIZE || len2 > REG_SIZE)  // ignore this one!
    return;

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

  if (len0 > REG_SIZE || len1 > REG_SIZE)  // ignore this one!
    return;

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

  if (len0 > REG_SIZE)  // ignore this one!
    return;

  static MsRecord k;
  k.r1.len = k.w0.len = 0;

  k.r0.addr = reinterpret_cast<ADDRINT>(addr0);
  k.r0.len = len0;

  PIN_SafeCopy(k.r0.content, static_cast<UINT8 *>(addr0), len0);

  rec_inst(ip, k);
}

static VOID recorder_w(ADDRINT ip, VOID *addr2, UINT32 len2) {
  if (!tracing_on) return;

  if (len2 > REG_SIZE)  // ignore this one!
    return;

  static MsRecord k;
  k.r0.len = k.r1.len = 0;

  k.w0.addr = reinterpret_cast<ADDRINT>(addr2);
  k.w0.len = len2;

  PIN_SafeCopy(k.w0.content, static_cast<UINT8 *>(addr2), len2);

  rec_inst(ip, k);
}

// Pin calls this function every time a new instruction is encountered
static VOID instrumentor(INS ins, VOID *v) {
  ADDRINT pc = INS_Address(ins);
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BeginInstruction, IARG_INST_PTR,
                 IARG_UINT32, pc, IARG_END);

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
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)EndInstruction, IARG_END);
}

static VOID Fini(INT32 code, VOID *v) {
  // close the file if it hasn't already been closed
  if (!output_file_closed) {
    fclose(out);
    output_file_closed = true;
  }
}

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
  std::cout << sizeof(ADDRINT) << " " << sizeof(THREADID) << std::endl;
  // Register function to be called to instrument instructions
  std::cout << sizeof(MsRecord) << std::endl;
  INS_AddInstrumentFunction(instrumentor, 0);

  // Register function to be called when the application exits
  PIN_AddFiniFunction(Fini, 0);

  std::cerr << "===============================================" << std::endl;
  std::cerr << "This application is instrumented by the MPR Trace Generator"
            << std::endl;
  std::cerr << "Trace saved in " << fileName << std::endl;
  std::cerr << "===============================================" << std::endl;

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
