// -*- mode: c++ -*-
#include <iostream>
#include <sstream>
#include <string>

// execvp
#include <unistd.h>

// waitpid
#include <sys/types.h>
#include <sys/wait.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <easylogging++.h>

#include "spec.h"

namespace runcpp {
  class ProcessRunner {
    Spec spec;
    int pid;
    void MountRootfs();

  public:
    ProcessRunner(Spec);
    void Start();
    void Terminate();
    void Exec();
    int Wait();
  };

  class Container {
    std::string id;
    std::string name;
    std::string container_path;

  public:
    Container(std::string);
    void Start();
    void Stop();
  };
}
