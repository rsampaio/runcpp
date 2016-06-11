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

      // set mount private recusively
      mount("", "/", "", MS_PRIVATE | MS_REC, "");
      mount(root.string().c_str(), root.string().c_str(), "", MS_BIND, "");

      // create old_root
      chdir(root.string().c_str());
      mkdir("old", 0755);

      if (syscall(SYS_pivot_root, root.string().c_str(),
                  fs::canonical("old", ec).string().c_str(), NULL) != 0 &&
          ec.value() != 0) {
        perror("syscall");
      }

      // change to the new root
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
      int flags = 0;
      std::string data;
      std::map<std::string, int> mnt_flag = {{"ro", MS_RDONLY},
                                             {"nosuid", MS_NOSUID},
                                             {"nodev", MS_NODEV},
                                             {"noexec", MS_NOEXEC},
                                             {"synchronous", MS_SYNCHRONOUS},
                                             {"remount", MS_REMOUNT},
                                             {"mandlock", MS_MANDLOCK},
                                             {"dirsync", MS_DIRSYNC},
                                             {"noatime", MS_NOATIME},
                                             {"nodiratime", MS_NODIRATIME},
                                             {"bind", MS_BIND},
                                             {"move", MS_MOVE},
                                             {"rec", MS_REC},
                                             {"silent", MS_SILENT},
                                             {"posixacl", MS_POSIXACL},
                                             {"unbindable", MS_UNBINDABLE},
                                             {"private", MS_PRIVATE},
                                             {"slave", MS_SLAVE},
                                             {"shared", MS_SHARED},
                                             {"relatime", MS_RELATIME},
                                             {"kernmount", MS_KERNMOUNT},
                                             {"i_version", MS_I_VERSION},
                                             {"strictatime", MS_STRICTATIME},
                                             {"lazytime", MS_LAZYTIME},
                                             {"active", MS_ACTIVE},
                                             {"nouser", MS_NOUSER}};
      for (auto &f : this->_spec.mounts) {
        data = "";
        for (auto &o : f.options) {
          if (o.find("=") == std::string::npos) {
            flags |= mnt_flag[o];
          } else {
            if (data != "") {
              data += ",";
            }
            data += o;
          }
        }

        fs::create_directories(fs::path(f.destination));
        mount(f.source.c_str(), f.destination.c_str(), f.type.c_str(), flags,
              data.c_str());
      }
    }

    void Container::set_hostname() {
      sethostname(this->_spec.hostname.c_str(), this->_spec.hostname.size());
    }

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
      // obj->set_network_interface();
      obj->set_hostname();

      obj->_process.pid = execvp(c_args[0], (char **)c_args);
      if (obj->_process.pid < 0) {
        perror("execvp");
      }
      return 0;
    }
  }
}
