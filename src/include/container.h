// -*- mode: c++ -*-
#include <cstdio>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>

#include <stdio.h>

// execvp,sethostname
#include <unistd.h>

// mount
#include <sys/mount.h>

// pivot_root
#include <sys/syscall.h>

#include <sys/socket.h>
#include <sys/types.h>

// clone
#define GNU_SOURCE 1
#include <sched.h>

// sleep
#include <chrono>
using namespace std::chrono_literals;

#include <thread>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <easylogging++.h>

// libstdc++ filebuf
#include <ext/stdio_filebuf.h>

#include "process.h"
#include "spec.h"

namespace runcpp {
  namespace container {
    class Container {
    private:
      const char **_c_args;
      char **_init_args;
      char **_init_env;
      std::string const _container_path;
      spec::Spec _spec;
      process::Process _process;

      void pivot_root();
      void mount_filesystems();
      void set_hostname();
      int clone();
      static int child_exec(void *);
      std::vector<int> new_pipe();

      std::map<std::string, int> _clone_flags = {{"pid", CLONE_NEWPID},
                                                 {"network", CLONE_NEWNET},
                                                 {"mount", CLONE_NEWNS},
                                                 {"ipc", CLONE_NEWIPC},
                                                 {"uts", CLONE_NEWUTS}};

      std::map<std::string, int> _mount_flags = {
          {"ro", MS_RDONLY},
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

    public:
      std::string id;
      Container(std::string, spec::Spec);
      int Start(process::Process);
      Container Load(std::string);
      void Destroy();
      static void InitContainer(int, int);
      // Processes
      // Stats
      // Pause
      // Resume
      // Signal
    };
  }
}
