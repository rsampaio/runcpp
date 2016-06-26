#include "process.h"

namespace runcpp {
  namespace process {
    Process::Process() : _stdin(0), _stdout(1), _stderr(2), pid(0) {}

    int Process::Wait() {
      int exit_status;
      waitpid(this->pid, &exit_status, 0);
      return exit_status;
    }
  }
}
