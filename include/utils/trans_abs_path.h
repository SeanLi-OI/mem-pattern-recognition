
#include <filesystem>

inline void transAbsolute(std::string &p) {
  if (!((std::filesystem::path)p).is_absolute())
    p = std::string(std::filesystem::current_path()) + "/" + p;
}
