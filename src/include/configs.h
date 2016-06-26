// -*- mode: c++ -*-
#include <string>
#include <vector>

namespace runcpp {
  namespace configs {
    // variadic-arguments c++11
    template <typename... Args>
    std::string string_fmt(const std::string &format, Args... args) {
      size_t sz = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
      std::vector<char> buf(sz);
      std::snprintf(&buf[0], buf.size(), format.c_str(), args...);
      return std::string(buf.begin(), buf.end());
    }

    class InitConfig {
    private:
      std::vector<std::string> args;
      std::vector<std::string> env;
      std::string cwd;
      std::string capabilities;
      std::string process_label;
      std::string apparmor_profile;
      bool no_new_privileges;
      std::string user;
      std::string console;
      int passed_files_count;
      std::string containerid;
      std::string start_pipe_path;
      // Config config;
      // std::vector<Network> networks;
      // std::vector<Rlimits> rlimits;
    };
  }
}
