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
      std::string id;
      spec::Spec spec;

    private:
      std::string const _container_path;

    public:
      Container(std::string, spec::Spec);
      void Start(process::Process);
      void Destroy();
      // Processes
      // Stats
      // Pause
      // Resume
      // Signal
    };
  }
}
