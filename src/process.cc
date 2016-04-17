#include "process.h"

namespace runcpp {
  namespace process {
    Process::Process() {}

    int Process::Wait() {
      int exit_status;
      waitpid(this->pid, &exit_status, 0);
      return exit_status;
    }
  }
}
