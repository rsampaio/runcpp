// -*- mode: c++ -*-
#include <string>
#include <vector>

// waitpid
#include <sys/types.h>
#include <sys/wait.h>

namespace runcpp {
  namespace process {
    class Process {
    public:
      std::vector<std::string> args;
      std::vector<std::string> env;
      std::string user;
      int stdin, stdout, stderr;
      int pid;

      Process();
      int Wait();
    };
  }
}
