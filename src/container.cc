#include "container.h"

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : spec(spec), id(id) {}

    void Container::Start(process::Process process) {
      int arg_size = process.args.size();
      const char **c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        c_args[i] = process.args[i].c_str();
      }

      c_args[arg_size] = NULL;
      process.pid = execvp(c_args[0], (char **)c_args);
    }

    void Container::Destroy() {
      std::cout << "stoping container: " << this->id << "\n";
    }
  }
}
