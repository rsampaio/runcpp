#include "process.h"

namespace runcpp {
  namespace process {
    Process::Process() : stdin(0), stdout(1), stderr(2), pid(0) {}

    int Process::Wait() {
      int exit_status;
      waitpid(this->pid, &exit_status, 0);
      return exit_status;
    }
  }
}
