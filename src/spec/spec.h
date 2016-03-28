// -*- mode: c++ -*-
#include <fstream>
#include <string>
#include <list>

#include "json.hpp"
using json = nlohmann::json;

namespace runcpp {
  namespace spec {
    struct Root {
      std::string path;
      bool readonly;
    };

    struct Resources {};
    struct Seccomp {};
    struct Device {};
    struct Namespace {};

    struct Linux {
      // UIDMappings []IDMapping
      // GIDMappings []IDMapping
      Seccomp seccomp;
      Resources resources;
      std::map<std::string, std::string> sysctl;
      std::list<Device> devices;
      std::list<Namespace> ns; // namespace is reserved of course
      std::string cgroups_path;
      std::string rootfs_propagation;
    };

    struct Mount {
      std::string destination;
      std::string type;
      std::string source;
      std::list<std::string> options;
    };

    struct Hook {
      std::string path;
      std::list<std::string> args;
      std::map<std::string, std::string> env;
      int *timeout;
    };

    struct Platform {
      std::string os;
      std::string arch;
    };

    struct Process {
      std::list<std::string> args;
      std::list<std::string> env;
      std::string cwd;
      std::string user;
      bool tty;
    };

    class Spec {
    public:
      Root root;
      Linux lnx; // lower case linux is defined
      Platform platform;

      std::map<std::string, std::string> annotations;
      std::list<Process> processes;
      std::list<Mount> mounts;
      std::list<Hook> hooks;
      std::string hostname;
      std::string version;

      Spec(std::string);
    };
  }
}
