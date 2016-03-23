// -*- mode: c++ -*-
#include <fstream>
#include <string>
#include <list>

#include "json.hpp"
using json = nlohmann::json;

namespace runcpp {
  namespace spec {

    class Platform {};

    class Process {
    public:
      std::list<std::string> args;
      std::list<std::string> env;
      std::string cwd;
      std::string user;
      bool tty;
    };

    class Spec {
    public:
      std::string version;
      spec::Platform platform;
      std::list<spec::Process> processes;

      Spec(std::string);
    };
  }
}
