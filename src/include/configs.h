// -*- mode: c++ -*-
#include <string>

namespace runcpp {
  namespace configs {

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
