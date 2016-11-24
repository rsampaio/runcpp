#include "container.h"
#include "configs.h"

namespace runcpp {
  namespace container {
    Container::Container(std::string id, spec::Spec spec)
        : id(id), _spec(spec),
          _logger(spdlog::stdout_logger_mt("container", true)) {}

    int Container::Start(process::Process process) {
      this->_process = process;
      return this->clone();
    }

    Container Container::Load(std::string id) {
      spec::Spec spec("");
      return Container(id, spec);
    }

    void Container::Destroy() {
      this->_logger->info("destroy container: {}", this->id);
    }

    void Container::InitContainer(int syncfd, int statefd) {
      int status, pid;
      std::vector<char> init_config(BUFSIZ);
      auto logger = spdlog::stdout_logger_mt("init", true);

      logger->info("read bootstrap data");

      // read synchronization data
      int sz = read(syncfd, &init_config[0], init_config.size());
      if (sz < 0) {
        logger->error("read bootstrap: {}", std::strerror(errno));
      }

      json init_config_json =
          json::parse(std::string(init_config.begin(), init_config.end()));

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
      auto logger = spdlog::stdout_logger_mt("sync", true);

      int sz =
          write(syncfd, std::to_string(runcpp::process::PROC_READY).c_str(), 1);
      if (sz < 0) {
        logger->error("fwrite from init error: {}", std::strerror(errno));
      }

      sz = read(syncfd, &buf[0], buf.size());
      if (sz < 0) {
        logger->error("fread from init error: {}", std::strerror(errno));
      }
    }

    void Container::pivot_root() {
      fs::path old_root, root_path, root;
      std::error_code ec;
      root_path =
          fs::path(this->_spec.spec_dir) / fs::path(this->_spec.root.path);
      old_root = root_path / fs::path("old");

      root = fs::canonical(root_path, ec);
      if (ec) {
        this->_logger->error(ec.message());
      }

      // set mount private recusively
      mount("", "/", "", MS_PRIVATE | MS_REC, "");
      mount(root.string().c_str(), root.string().c_str(), "", MS_BIND, "");

      // create old_root
      fs::current_path(root);
      fs::create_directory("old", ec); // ignore error
      fs::permissions("old", fs::perms::remove_perms | fs::perms::others_all);

      if (syscall(SYS_pivot_root, root.string().c_str(),
                  fs::canonical("old", ec).string().c_str(), NULL) != 0 &&
          ec) {
        this->_logger->error("pivot_root: {}", std::strerror(errno));
      }

      // change to the new root
      fs::current_path(fs::path("/"), ec);
      if (ec) {
        this->_logger->error("chdir: {}", std::strerror(errno));
      }

      // relative old root
      mount("", "/old", "", MS_PRIVATE | MS_REC, "");
      if (umount2("/old", MNT_DETACH) != 0) {
        this->_logger->error("umount2: {}", std::strerror(errno));
      }

      fs::remove_all(old_root, ec); // ignore error
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
        this->_logger->error("child stack: {}", std::strerror(errno));
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
          runcpp::configs::string_fmt("_RUNCPP_INITPIPE=%d", fds[1]);
      const char *env[] = {init_pipe_env.c_str(), NULL};
      const char *args[] = {"/proc/self/exec", "init", NULL};
      this->_init_env = (char **)env;
      this->_init_args = (char **)args;

      // clone syscall
      pid = ::clone(&Container::child_exec, stack_top, flags | SIGCHLD, this);
      if (pid == -1) {
        this->_logger->error("clone: {}", std::strerror(errno));
        exit(-1);
      }

      // read init config from pipe first
      json init_config_json;
      init_config_json["args"] = this->_process.args;
      init_config_json["env"] = this->_process.env;
      std::string json_string = init_config_json.dump() + "\n";

      int sz = write(fds[0], json_string.c_str(), json_string.size());
      if (sz < 0) {
        this->_logger->error("write: {}", std::strerror(errno));
      }

      // sync with child
      this->sync_child(fds[0]);

      return waitpid(pid, &status, 0);
    }

    void Container::sync_child(int fd) {
      std::vector<char> buf(BUFSIZ);
      int sz = read(fd, &buf[0], buf.size());
      if (sz < 0) {
        std::cerr << "sync read error";
        exit(-1);
      }

      if (std::stoi(std::string(buf.begin(), buf.end())) !=
          runcpp::process::PROC_READY) {
        this->_logger->error("synchronization flag invalid: {}",
                             std::string(buf.begin(), buf.end()));
      }

      sz = write(fd, std::to_string(runcpp::process::PROC_RUN).c_str(), 1);
      if (sz < 0) {
        this->_logger->error("fwrite: {}", std::strerror(errno));
      }
    }

    std::vector<int> Container::new_pipe() {
      std::vector<int> pipe_pair;
      int pipe_fds[2];
      if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipe_fds) != 0) {
        this->_logger->error("socketpair: {}", std::strerror(errno));
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
      obj->_process.pid = execvpe(obj->_init_args[0], (char **)obj->_init_args,
                                  (char **)obj->_init_env);

      if (obj->_process.pid < 0) {
        obj->_logger->error("execvpe: {}", std::strerror(errno));
        exit(-1);
      }
      if (obj->pid_file != "") {
        fs::create_directories(fs::path(obj->pid_file).parent_path());
        std::FILE *fp = std::fopen(obj->pid_file.c_str(), "w+");
        if (fp == nullptr) {
          obj->_logger->error("fopen: {}", std::strerror(errno));
        }
        std::string pid_string(std::to_string(obj->_process.pid));
        std::fwrite(pid_string.c_str(), sizeof(char), pid_string.size(), fp);
        std::fclose(fp);
      }
      return 0;
    }
  }
}
