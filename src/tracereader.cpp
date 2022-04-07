#include "tracereader.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

tracereader::tracereader(uint8_t cpu, std::string _ts)
    : cpu(cpu), trace_string(_ts) {
  std::string last_dot = trace_string.substr(trace_string.find_last_of("."));

  if (trace_string.substr(0, 4) == "http") {
    // Check file exists
    char testfile_command[4096];
    sprintf(testfile_command, "wget -q --spider %s", trace_string.c_str());
    FILE *testfile = popen(testfile_command, "r");
    if (pclose(testfile)) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "wget -qO- -o /dev/null %2$s | %1$s -dc";
  } else {
    std::ifstream testfile(trace_string);
    if (!testfile.good()) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "%1$s -dc %2$s";
  }

  if (last_dot[1] == 'g')  // gzip format
    decomp_program = "gzip";
  else if (last_dot[1] == 'x')  // xz
    decomp_program = "xz";
  else {
    // std::cout
    //     << "MPR does not support traces other than gz or xz compression!"
    //     << std::endl;
    // assert(0);
    open_raw(trace_string);
    return;
  }

  open(trace_string);
}

tracereader::~tracereader() { close(); }

void tracereader::read_str(std::string &str) {
  size_t len;
  fread(&len, sizeof(size_t), 1, trace_file);
  if (len > 0) {
    str.resize(len);
    fread(&str[0], len, 1, trace_file);
  } else {
    str = "";
  }
}

MyInstr tracereader::read_single_instr(bool &isend) {
  MsRecord trace_read_instr;
  while (!fread(&trace_read_instr, sizeof(MsRecord), 1, trace_file)) {
    // reached end of file for this trace
    std::cout << "*** Reached end of trace: " << trace_string << std::endl;
    // close the trace file and re-open it
    close();
    isend = true;
    break;
    // open(trace_string);
  }
  auto ret = MyInstr(trace_read_instr);
  if (!isend) {
    read_str(ret.func_name);
    read_str(ret.image);
  }

  // copy the instruction into the performance model's instruction format
  return ret;
}

void tracereader::open_raw(std::string trace_string) {
  trace_file = fopen(trace_string.c_str(), "rb");
  if (trace_file == NULL) {
    std::cerr << std::endl
              << "*** CANNOT OPEN TRACE FILE: " << trace_string << " ***"
              << std::endl;
    assert(0);
  }
}

void tracereader::open(std::string trace_string) {
  char gunzip_command[4096];
  sprintf(gunzip_command, cmd_fmtstr.c_str(), decomp_program.c_str(),
          trace_string.c_str());
  trace_file = popen(gunzip_command, "r");
  if (trace_file == NULL) {
    std::cerr << std::endl
              << "*** CANNOT OPEN TRACE FILE: " << trace_string << " ***"
              << std::endl;
    assert(0);
  }
}

void tracereader::close() {
  if (trace_file != NULL) {
    pclose(trace_file);
  }
}

class input_tracereader : public tracereader {
  MyInstr last_instr;
  bool initialized = false;

 public:
  input_tracereader(uint8_t cpu, std::string _tn) : tracereader(cpu, _tn) {}

  MyInstr get(bool &isend) {
    auto trace_read_instr = read_single_instr(isend);

    if (!initialized) {
      // last_instr = trace_read_instr;
      initialized = true;
    }

    // MsRecord retval = last_instr;

    // last_instr = trace_read_instr;
    // return retval;
    return trace_read_instr;
  }
};

tracereader *get_tracereader(std::string fname, uint8_t cpu,
                             bool is_cloudsuite) {
  return new input_tracereader(cpu, fname);
}
