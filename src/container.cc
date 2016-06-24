#include "container.h"
#include "configs.h"

// variadic-arguments c++11
template <typename... Args>
std::string format(const std::string &format, Args... args) {
  size_t sz = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::vector<char> buf(sz);
  std::snprintf(&buf[0], buf.size(), format.c_str(), args...);
  return std::string(buf.begin(), buf.end());
}

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : id(id), _spec(spec) {}

    int Container::Start(process::Process process) {
      this->_process = process;
      return this->clone();
    }

    Container Container::Load(std::string id) {
      spec::Spec spec("");
      return Container(id, spec);
    }

    void Container::Destroy() {
      LOG(INFO) << "destroy container: " << this->id;
    }

    void Container::InitContainer(int syncfd, int statefd) {
      json init_config_json;
      std::string buf;

      // non-portable file descriptor to stream
      __gnu_cxx::stdio_filebuf<char> filebuf(syncfd, std::ios::in);
      std::istream pipe_stream(&filebuf);
      std::getline(pipe_stream, buf);

      // stream from the buffer to json parser
      std::istringstream(buf) >> init_config_json;
      LOG(INFO) << init_config_json;
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

      for (auto &f : this->_spec.mounts) {
        data = "";
        for (auto &o : f.options) {
          if (o.find("=") == std::string::npos) {
            flags |= this->_mount_flags[o];
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

      // convert args
      int arg_size = this->_process.args.size();
      this->_c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        this->_c_args[i] = this->_process.args[i].c_str();
      }

      this->_c_args[arg_size] = NULL;

      // child stack allocation
      stack = (char *)malloc(1024 * 1024);
      if (stack == NULL) {
        perror("malloc");
      }

      stack_top = stack + (1024 * 1024);

      // namespaces
      flags = 0;
      for (auto &n : this->_spec.spec_linux.namespaces) {
        flags |= this->_clone_flags[n.ns_type];
      }

      std::vector<int> fds = this->new_pipe();
      std::string init_pipe_env = format("_LIBCONTAINER_INITPIPE=%d", fds[1]);
      const char *env[] = {init_pipe_env.c_str(), NULL};
      const char *args[] = {"/proc/self/exec", "init", NULL};
      this->_init_env = (char **)env;
      this->_init_args = (char **)args;

      // clone syscall
      pid = ::clone(&Container::child_exec, stack_top, flags | SIGCHLD, this);
      if (pid == -1) {
        perror("clone error");
      }

      std::string proc_sync("{\"ok\": true}");

      // non-portable way of turning an fd into a stream
      __gnu_cxx::stdio_filebuf<char> filebuf(fds[0], std::ios::out);
      std::ostream pipe_stream(&filebuf);
      pipe_stream << std::istringstream(proc_sync).rdbuf();
      pipe_stream << std::endl;
      pid = waitpid(pid, &status, 0);
      return pid;
    }

    std::vector<int> Container::new_pipe() {
      std::vector<int> pipe_pair;
      int pipe_fds[2];
      if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipe_fds) != 0) {
        perror("socketpair");
      }
      pipe_pair.push_back(pipe_fds[0]);
      pipe_pair.push_back(pipe_fds[1]);
      return pipe_pair;
    }

    int Container::child_exec(void *arg) {
      Container *obj = static_cast<Container *>(arg);
      // container setup
      obj->pivot_root();
      obj->mount_filesystems();
      // obj->set_network_interface();
      obj->set_hostname();
      obj->_process.pid = execvpe("/proc/self/exe", (char **)obj->_init_args,
                                  (char **)obj->_init_env);
      if (obj->_process.pid < 0) {
        perror("execvp");
      }

      return 0;
    }
  }
}
