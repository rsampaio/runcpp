// -*- mode: c++ -*-
#include <iostream>
#include <sstream>
#include <string>

// execvp
#include <unistd.h>

// mount
#include <sys/mount.h>

#include <easylogging++.h>

#include "spec.h"
#include "process.h"

namespace runcpp {
  namespace container {
    class Container {
    private:
      std::string const _container_path;
      spec::Spec _spec;

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
