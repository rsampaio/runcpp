// -*- mode: c++ -*-
#include <string>
#include <vector>

// waitpid
#include <sys/types.h>
#include <sys/wait.h>

namespace runcpp {
  namespace process {
    enum SyncState : int {
      PROC_READY = 0,
      PROC_ERROR,
      PROC_RUN,
      PROC_HOOKS,
      PROC_RESUME
    };

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
