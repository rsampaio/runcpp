#include "container.h"

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : id(id), _spec(spec) {}

    int Container::Start(process::Process process) {
      int arg_size = process.args.size();
      const char **c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        c_args[i] = process.args[i].c_str();
      }

      c_args[arg_size] = NULL;
      process.pid = execvp(c_args[0], (char **)c_args);
      if (process.pid < 0) {
        return errno;
      }
      return 0;
    }

    Container Container::Load(std::string id) {
      spec::Spec spec("");
      return Container(id, spec);
    }

    void Container::Destroy() {
      LOG(INFO) << "destroy container: " << this->id;
    }
  }
}
