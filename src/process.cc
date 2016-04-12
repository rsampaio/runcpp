#include "container.h"

using namespace std::chrono_literals;

namespace runcpp {
  ProcessRunner::ProcessRunner(Spec spec) : spec(spec) {}

  void ProcessRunner::Start() {
    this->MountRootfs();
    this->Exec();
  }

  int ProcessRunner::Wait() {
    int exit_status;
    waitpid(this->pid, &exit_status, 0);
    return exit_status;
  }

  void ProcessRunner::Exec() {
    int arg_size = this->spec.process.args.size();
    const char **c_args = new const char *[arg_size + 1];

    for (int i = 0; i < arg_size; i++) {
      c_args[i] = this->spec.process.args[i].c_str();
    }
    c_args[arg_size] = NULL;
    this->pid = execvp(c_args[0], (char **)c_args);
  }

  void ProcessRunner::MountRootfs() {}
}
