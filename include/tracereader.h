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
  size_t has_read;
  size_t total_len;

  void read_str(std::string& str);
  size_t file_size2(const char* filename);
  void print_progress(size_t current, size_t all);

 public:
  tracereader(const tracereader& other) = delete;
  tracereader(uint8_t cpu, std::string _ts);
  ~tracereader();
  void open(std::string trace_string);
  void open_raw(std::string trace_string);
  void close();

  MyInstr read_single_instr(bool& isend);

  virtual MyInstr get(bool& isend) = 0;
};

tracereader* get_tracereader(std::string fname, uint8_t cpu,
                             bool is_cloudsuite);
