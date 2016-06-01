#include "container.h"

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : id(id), _spec(spec) {}

    int Container::Start(process::Process process) {
      this->_process = process;

      int clone_pid = this->clone();
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
      fs::path old_root, root_path, root;
      std::error_code ec;
      root_path =
          fs::path(this->_spec.spec_dir) / fs::path(this->_spec.root.path);
      old_root = root_path / fs::path("old");

      root = fs::canonical(root_path, ec);
      if (ec.value() != 0) {
        LOG(ERROR) << ec.message();
      }

      mount("", "/", "", MS_PRIVATE | MS_REC, "");
      mount(root.string().c_str(), root.string().c_str(), "", MS_BIND, "");

      chdir(root.string().c_str());
      mkdir("old", 0755);

      if (syscall(SYS_pivot_root, root.string().c_str(),
                  fs::canonical("old", ec).string().c_str(), NULL) != 0 &&
          ec.value() != 0) {
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

    void Container::mount_filesystems() {
      LOG(INFO) << "mount_filesystems";
      for (auto &f : this->_spec.mounts) {
        LOG(INFO) << f.type;
        for (auto &o : f.options) {
          LOG(INFO) << "\t" << o;
        }
        mount(f.source.c_str(), f.destination.c_str(), f.type.c_str(),
              MS_NOSUID, "");
      }
    }

    void Container::set_hostname() { sethostname(this->id.c_str(), 8); }

    int Container::clone() {
      char *stack;
      char *stack_top;
      int pid, status, flags;

      std::map<std::string, int> clone_flags = {{"pid", CLONE_NEWPID},
                                                {"network", CLONE_NEWNET},
                                                {"mount", CLONE_NEWNS},
                                                {"ipc", CLONE_NEWIPC},
                                                {"uts", CLONE_NEWUTS}};

      // child stack allocation
      stack = (char *)malloc(1024 * 1024);
      if (stack == NULL) {
        perror("malloc");
      }

      stack_top = stack + (1024 * 1024);

      // namespaces
      flags = 0;
      for (auto &n : this->_spec.spec_linux.namespaces) {
        flags |= clone_flags[n.ns_type];
      }

      // clone syscall
      pid = ::clone(&Container::child_exec, stack_top, flags | SIGCHLD, this);
      if (pid == -1) {
        perror("clone error");
      }

      // wait
      std::this_thread::sleep_for(1s);
      pid = waitpid(pid, &status, 0);
      return pid;
    }

    int Container::child_exec(void *arg) {
      Container *obj = static_cast<Container *>(arg);

      // convert args
      int arg_size = obj->_process.args.size();
      const char **c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        c_args[i] = obj->_process.args[i].c_str();
      }
      c_args[arg_size] = NULL;

      // container setup
      obj->pivot_root();
      obj->mount_filesystems();
      // obj->set_network_ns();
      obj->set_hostname();

      obj->_process.pid = execvp(c_args[0], (char **)c_args);
      if (obj->_process.pid < 0) {
        perror("execvp");
      }
      return 0;
    }
  }
}
