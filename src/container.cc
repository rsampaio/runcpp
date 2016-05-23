#include "container.h"

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : id(id), _spec(spec) {}

    int Container::Start(process::Process process) {
      int arg_size = this->_process.args.size();
      const char **c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        c_args[i] = this->_process.args[i].c_str();
      }

      c_args[arg_size] = NULL;

      this->_process = process;

      std::jmp_buf jp;
      if (setjmp(jp) == 1) {
        LOG(INFO) << "child process";
        this->pivot_root();
        this->_process.pid = execvp(c_args[0], (char **)c_args);
        if (this->_process.pid < 0) {
          perror("execvp");
        }
      }

      int clone_pid = this->clone(jp);
      return clone_pid;
    }

    Container Container::Load(std::string id) {
      spec::Spec spec("");
      return Container(id, spec);
    }

    void Container::Destroy() {
      LOG(INFO) << "destroy container: " << this->id;
    }

    void Container::pivot_root() {
      fs::path old_root, root;
      old_root = fs::path(this->_spec.spec_dir) /
                 fs::path(this->_spec.root.path) / fs::path("old");

      root = fs::canonical(fs::path(this->_spec.spec_dir) /
                           fs::path(this->_spec.root.path));
      LOG(INFO) << old_root.string();

      mount("", "/", "", MS_PRIVATE | MS_REC, "");
      mount(root.string().c_str(), root.string().c_str(), "", MS_BIND, "");

      chdir(root.string().c_str());
      mkdir("old", 0755);

      if (syscall(SYS_pivot_root, root.string().c_str(),
                  fs::canonical("old").string().c_str(), NULL) != 0) {
        perror("syscall");
      }
      if (chdir("/") != 0) {
        perror("chdir");
      }

      // relative old root
      mount("", "/old", "", MS_PRIVATE | MS_REC, "");

      if (umount2("/old", MNT_DETACH) != 0) {
        perror("unmount2");
      }
      rmdir(old_root.c_str());
    }

    int Container::clone(std::jmp_buf jp) {
      char *stack;
      char *stackTop;
      int pid;

      // child stack allocation
      stack = (char *)malloc(1024 * 1024);
      if (stack == NULL)
        perror("malloc");

      stackTop = stack + (1024 * 1024);
      pid = ::clone(&Container::child_func, stackTop,
                    CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD, &jp);
      if (pid == -1) {
        perror("clone error");
      }

      LOG(INFO) << "clone returned PID=" << pid << "\n";
      return pid;
    }
  }
}
