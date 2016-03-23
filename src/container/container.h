// -*- mode: c++ -*-
#include <iostream>
#include <string>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "spec/spec.h"

namespace runcpp {
  namespace container {
    class Container {
      std::string id;
      std::string name;
      std::string container_path;

    public:
      Container(std::string);
      void Start();
      void Stop();
      void Exec(std::string cmd);
    };
  }
}
