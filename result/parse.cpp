#include <assert.h>

#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

// Execute shell command and get stdout
std::string exec(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

// Int to Hex String
template <typename T>
std::string int_to_hex(T i) {
  std::stringstream stream;
  stream << std::hex << i;
  return stream.str();
}

// PATTERN constant variable
const int PATTERN_NUM = 8;
const std::string PATTERN_NAME[] = {"fresh",    "static",   "stride",
                                    "pointerA", "pointerB", "indirect",
                                    "chain",    "other"};
enum PATTERN : uint16_t {
  FRESH,
  STATIC,
  STRIDE,
  POINTER_A,
  POINTER_B,
  INDIRECT,
  CHAIN,
  OTHER
};
static std::unordered_map<std::string, PATTERN> const table = {
    {"fresh", PATTERN::FRESH},        {"static", PATTERN::STATIC},
    {"stride", PATTERN::STRIDE},      {"pointerA", PATTERN::POINTER_A},
    {"pointerB", PATTERN::POINTER_B}, {"indirect", PATTERN::INDIRECT},
    {"chain", PATTERN::CHAIN},        {"other", PATTERN::OTHER}};
template <typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

// PC Metadata
struct pcmeta {
  PATTERN pattern;
  int count;
  int miss_count;
  pcmeta() {}
  pcmeta(std::string str, int cnt) {
    auto it = table.find(str);
    assert(it != table.end());
    pattern = table.find(str)->second;
    count = cnt;
    miss_count = 0;
  }
};
std::unordered_map<unsigned long long, pcmeta> pc2meta;

std::string get_file_line(const char file_name[], int line_no) {
  std::ifstream fin(file_name);
  std::string s;
  assert(fin);
  line_no--;
  while (getline(fin, s) && line_no) line_no--;
  assert(line_no == 0);
  return s;
}
std::string pc2line(char bin_file[], unsigned long long pc) {
  auto ret =
      exec(("addr2line -Cfie " + std::string(bin_file) + " " + int_to_hex(pc))
               .c_str());
  if (ret[0] == '?') return "";
  int len = ret.length(), offset1, offset2, offset3;
  for (offset1 = 0; offset1 < len && ret[offset1] != '\n'; offset1++)
    ;
  assert(offset1 < len);
  for (offset2 = ++offset1; offset2 < len && ret[offset2] != ':'; offset2++)
    ;
  assert(offset2 < len);
  for (offset3 = ++offset2; offset3 < len && isdigit(ret[offset3]); offset3++)
    ;
  std::string file_name = ret.substr(offset1, offset2 - offset1 - 1);
  std::string line_no = ret.substr(offset2, offset3 - offset2);
  if (file_name[0] == '?' || line_no.length() == 0) return "";
  return get_file_line(file_name.c_str(), std::stoi(line_no));
}

int main(int argc, char* argv[]) {
  std::fstream in1, in2, out;
  assert(argc > 4);
  in1.open(argv[1], std::ios::in);
  assert(in1);
  in2.open(argv[2], std::ios::in);
  assert(in2);

  unsigned long long pc, addr;
  std::string pattern;
  int count, instr_id;
  while (in2 >> std::hex >> pc >> pattern >> std::dec >> count) {
    pc2meta[pc] = pcmeta(pattern, count);
  }
  while (in1 >> std::dec >> instr_id >> std::hex >> pc >> std::hex >> addr) {
    auto it = pc2meta.find(pc);
    if (it == pc2meta.end()) {
      std::cerr << "Cannot find PC:" << std::hex << pc << " in " << argv[2]
                << std::endl;
      continue;
    }
    assert(it != pc2meta.end());
    it->second.miss_count++;
  }
  in1.close();
  in2.close();
  out.open(argv[3], std::ios::out);
  assert(out);
  for (auto meta : pc2meta) {
    auto ret = pc2line(argv[4], meta.first);
    out << std::hex << meta.first << ",," << std::dec << meta.second.count << ",,"
        << std::dec << meta.second.miss_count << ",,"
        << PATTERN_NAME[to_underlying(meta.second.pattern)] << ",," << ret
        << std::endl;
  }
  return 0;
}