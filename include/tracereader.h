#include <cstdio>
#include <string>

#include "instruction.h"

class tracereader {
 protected:
  FILE* trace_file = NULL;
  uint8_t cpu;
  std::string cmd_fmtstr;
  std::string decomp_program;
  std::string trace_string;

 public:
  tracereader(const tracereader& other) = delete;
  tracereader(uint8_t cpu, std::string _ts);
  ~tracereader();
  void open(std::string trace_string);
  void close();

  template <typename T>
  T read_single_instr(bool& isend);

  virtual input_instr get(bool& isend) = 0;
};

tracereader* get_tracereader(std::string fname, uint8_t cpu,
                             bool is_cloudsuite);
