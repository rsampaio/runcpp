// -*- mode: c++ -*-
#include <iostream>
#include <sstream>
#include <string>

// execvp,sethostname
#include <unistd.h>

// mount
#include <sys/mount.h>

// pivot_root
#include <sys/syscall.h>

// clone
#define GNU_SOURCE 1
#include <sched.h>

// setjmp/longjmp
#include <csetjmp>

// sleep
#include <chrono>
using namespace std::chrono_literals;

#include <thread>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <easylogging++.h>

#include "process.h"
#include "spec.h"

namespace runcpp {
  namespace container {
    class Container {
    private:
      std::string const _container_path;
      spec::Spec _spec;
      process::Process _process;

      void pivot_root();
      void mount_filesystems();
      void set_hostname();
      int clone();
      static int child_exec(void *);

    public:
      std::string id;
      Container(std::string, spec::Spec);
      int Start(process::Process);
      Container Load(std::string);
      void Destroy();
      // Processes
      // Stats
      // Pause
      // Resume
      // Signal
    };
  }
}
