
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <string>

#include "pin.H"

typedef struct trace_instr_format {
  unsigned long long int ip;  // instruction pointer (program counter) value

  unsigned long long int address;
  unsigned long long int value;
  bool isWrite;
} trace_instr_format_t;

/* ================================================================== */
// Global variables
/* ================================================================== */

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

unsigned long long int destination_memory[NUM_INSTR_DESTINATIONS];
unsigned long long int source_memory[NUM_INSTR_SOURCES];

UINT64 instrCount = 0;

FILE *out;

PIN_LOCK globalLock;

bool output_file_closed = false;
bool tracing_on = false;

bool hasWrite = false;
VOID *writeAddr;
uint64_t writeIP;
UINT32 writeSize;
UINT32 mem_ops;

uint64_t curr_ip;

trace_instr_format_t curr_instr;

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
  curr_ip = (unsigned long long)ip;

//   fprintf(stderr, "PC: %llu MemOps: %u\n", (unsigned long long)ip,
//           (unsigned)mem_ops);

  if (!tracing_on) return;
}

void EndInstruction() {
  // printf("%d]\n", (int)instrCount);

  // printf("\n");

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

void dump(uint64_t pc, VOID *addr, uint64_t value, bool isWrite) {
  curr_instr.ip = pc;
  curr_instr.address = (unsigned long long)addr;
  curr_instr.value = value;
  curr_instr.isWrite = isWrite;
  fwrite(&curr_instr, sizeof(trace_instr_format_t), 1, out);
  //   if(10>(unsigned long long )value&&0<(unsigned long long )value)
  //   fprintf(stderr, "%s: ADDR, VAL: %llx, %llu\n",isWrite?"W":"R", (unsigned
  //   long long int)addr, (unsigned long long int)value);
}

void doneWrite() {
  PIN_GetLock(&globalLock, 1);
  ADDRINT *addr_ptr = (ADDRINT *)writeAddr;
  ADDRINT value = 0;
//   fprintf(stderr, "W: ADDR, VAL: %llx, ", (unsigned long long int)writeAddr);
  if (addr_ptr != nullptr && addr_ptr != NULL && addr_ptr > 0) {
    PIN_SafeCopy(&value, addr_ptr, writeSize);
  }
//   fprintf(stderr, "%llu\n", (unsigned long long int)value);
  PIN_ReleaseLock(&globalLock);
  dump(writeIP, writeAddr, value, 1);
  hasWrite = false;
}

void MemoryRead(VOID *addr, UINT32 index, UINT32 read_size) {
  if (hasWrite) doneWrite();
  if (!tracing_on) return;

  // printf("0x%llx,%u ", (unsigned long long int)addr, read_size);
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (source_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      PIN_GetLock(&globalLock, 1);
      ADDRINT *addr_ptr = (ADDRINT *)addr;
      ADDRINT value = 0;
    //   fprintf(stderr, "R: ADDR, VAL: %llx, ", (unsigned long long int)addr);
      if (addr_ptr != nullptr && addr_ptr != NULL && addr_ptr > 0) {
        PIN_SafeCopy(&value, addr_ptr, read_size);
      }
    //   fprintf(stderr, "%llu\n", (unsigned long long int)value);
      PIN_ReleaseLock(&globalLock);
      dump(curr_ip, addr, value, 0);
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (source_memory[i] == 0) {
        source_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
  //   fprintf(stderr, "Read: ADDR, VAL: %lx, %lu\n", (unsigned long int)addr,
  //           (unsigned long int)value);
}

void MemoryWrite(VOID *addr, UINT32 index, UINT32 write_size) {
  if (hasWrite) doneWrite();
  if (!tracing_on) return;

  // printf("(0x%llx) ", (unsigned long long int) addr);

  // check to see if this memory write location is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (destination_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      writeAddr = addr;
      writeIP = curr_ip;
      writeSize = write_size;
      hasWrite = true;
    }
    break;
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (destination_memory[i] == 0) {
        destination_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {
  // instrument memory reads and writes
  UINT32 memOperands = INS_MemoryOperandCount(ins);
  mem_ops = memOperands;
  // begin each instruction with this function
  UINT32 opcode = INS_Opcode(ins);
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BeginInstruction, IARG_INST_PTR,
                 IARG_UINT32, opcode, IARG_END);

  // Iterate over each memory operand of the instruction.
  if (memOperands != 0) {
    UINT32 memOp = 0;
    //   for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      UINT32 read_size = INS_MemoryOperandSize(ins, memOp);

      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemoryRead, IARG_MEMORYOP_EA,
                     memOp, IARG_UINT32, memOp, IARG_UINT32, read_size,
                     IARG_END);
    }
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      UINT32 write_size = INS_MemoryOperandSize(ins, memOp);

      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemoryWrite, IARG_MEMORYOP_EA,
                     memOp, IARG_UINT32, memOp, IARG_UINT32, write_size,
                     IARG_END);
    }
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

  const char *fileName = argv[argc - 1];

  out = fopen(fileName, "ab");
  if (!out) {
    std::cout << "Couldn't open output trace file. Exiting." << std::endl;
    exit(1);
  }

  // Register function to be called to instrument instructions
  INS_AddInstrumentFunction(Instruction, 0);

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
