#include "container.h"
#include "configs.h"

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
      int status, pid;
      std::string init_config;

      // non-portable file descriptor to stream
      __gnu_cxx::stdio_filebuf<char> filebuf(syncfd, std::ios::in);
      std::istream pipe_stream(&filebuf);

      // pipe_stream >> init_config_json;
      std::getline(pipe_stream, init_config);
      json init_config_json = json::parse(init_config);
      LOG(INFO) << init_config_json["args"];

      runcpp::container::Container::sync_parent(syncfd);

      // convert args
      int arg_size = init_config_json["args"].size();
      const char **c_args = new const char *[arg_size + 1];
      for (int i = 0; i < arg_size; i++) {
        c_args[i] = init_config_json["args"][i].get<std::string>().c_str();
      }

      c_args[arg_size] = NULL;

      // convert env
      int env_size = init_config_json["env"].size();
      const char **c_env = new const char *[env_size + 1];
      for (int i = 0; i < env_size; i++) {
        c_env[i] = init_config_json["env"][i].get<std::string>().c_str();
      }

      c_env[env_size] = NULL;

      clearenv();
      pid = execvpe(c_args[0], (char **)c_args, (char **)c_env);
      waitpid(pid, &status, 0);
    }

    void Container::sync_parent(int syncfd) {
      std::vector<char> buf(1);
      int sz =
          write(syncfd, std::to_string(runcpp::process::PROC_READY).c_str(), 1);
      if (sz < 0) {
        LOG(ERROR) << "fwrite from init error: " << strerror(errno);
      }

      sz = read(syncfd, &buf[0], buf.size());
      if (sz < 0) {
        LOG(ERROR) << "fread from init error: ", strerror(errno);
      }
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
        LOG(ERROR) << "pivot_root: " << strerror(errno);
      }

      // change to the new root
      if (chdir("/") != 0) {
        LOG(ERROR) << "chdir: " << strerror(errno);
      }

      // relative old root
      mount("", "/old", "", MS_PRIVATE | MS_REC, "");
      if (umount2("/old", MNT_DETACH) != 0) {
        LOG(ERROR) << "unmount2: " << strerror(errno);
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
            flags |= this->_mnt_flags[o];
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

      // child stack allocation
      stack = (char *)malloc(1024 * 1024);
      if (stack == NULL) {
        LOG(ERROR) << strerror(errno);
        exit(-1);
      }

      stack_top = stack + (1024 * 1024);

      // namespaces
      flags = 0;
      for (auto &n : this->_spec.spec_linux.namespaces) {
        flags |= this->_clone_flags[n.ns_type];
      }

      std::vector<int> fds = this->new_pipe();
      std::string init_pipe_env =
          runcpp::configs::string_fmt("_LIBCONTAINER_INITPIPE=%d", fds[1]);
      const char *env[] = {init_pipe_env.c_str(), NULL};
      const char *args[] = {"/proc/self/exec", "init", NULL};
      this->_init_env = (char **)env;
      this->_init_args = (char **)args;

      // clone syscall
      pid = ::clone(&Container::child_exec, stack_top, flags | SIGCHLD, this);
      if (pid == -1) {
        LOG(ERROR) << "clone: " << strerror(errno);
        exit(-1);
      }

      // non-portable way of turning a fd into a stream
      __gnu_cxx::stdio_filebuf<char> filebuf(fds[0], std::ios::out);
      std::ostream pipe_stream(&filebuf);

      // read init config from pipe first
      json init_config_json;
      init_config_json["args"] = this->_process.args;
      init_config_json["env"] = this->_process.env;
      LOG(INFO) << init_config_json.dump();
      pipe_stream << init_config_json.dump() << std::endl;

      // sync with child
      this->sync_child(fds[0]);

      return waitpid(pid, &status, 0);
    }

    void Container::sync_child(int fd) {
      std::vector<char> buf(BUFSIZ);
      int sz = read(fd, &buf[0], buf.size());
      if (sz < 0) {
        LOG(ERROR) << "sync read error";
      }

      if (std::stoi(std::string(buf.begin(), buf.end())) !=
          runcpp::process::PROC_READY) {
        LOG(ERROR) << "synchronization flag invalid: "
                   << std::string(buf.begin(), buf.end());
      }

      sz = write(fd, std::to_string(runcpp::process::PROC_RUN).c_str(), 1);
      if (sz < 0) {
        LOG(ERROR) << "fwrite: " << strerror(errno);
      }
    }

    std::vector<int> Container::new_pipe() {
      std::vector<int> pipe_pair;
      int pipe_fds[2];
      if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipe_fds) != 0) {
        LOG(ERROR) << "socketpair: " << strerror(errno);
        exit(-1);
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
        LOG(ERROR) << "execvpe: " << strerror(errno);
        exit(-1);
      }

      return 0;
    }
  }
}
