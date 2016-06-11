// -*- mode: c++ -*-
#include <fstream>
#include <list>
#include <string>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

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
    struct Namespace {
      std::string ns_type; // type is reserved of course
      std::string path;
    };
    struct IDMappings {
      int host_id;
      int container_id;
      int size;
    };

    struct Linux {
      Seccomp seccomp;
      Resources resources;
      std::map<std::string, std::string> sysctl;
      std::vector<IDMappings> uid_mappings;
      std::vector<IDMappings> gid_mappings;
      std::vector<Device> devices;
      std::vector<Namespace> namespaces; // namespace is reserved of course
      std::string cgroups_path;
      std::string rootfs_propagation;
    };

    struct Mount {
      std::string destination;
      std::string type;
      std::string source;
      std::vector<std::string> options;
    };

    struct Hook {
      std::string path;
      std::vector<std::string> args;
      std::map<std::string, std::string> env;
      int *timeout;
    };

    struct Platform {
      std::string os;
      std::string arch;
    };

    struct User {
      int uid, gid;
      std::vector<int> additional_gids;
    };

    struct Process {
      std::vector<std::string> args;
      std::vector<std::string> env;
      std::string cwd;
      User user;
      bool terminal;
    };

    class Spec {
    public:
      Root root;
      Linux spec_linux; // lower case linux is defined
      Platform platform;
      Process process;
      std::map<std::string, std::string> annotations;
      std::vector<Mount> mounts;
      std::vector<Hook> hooks;
      std::string hostname;
      std::string oci_version;
      std::string spec_dir;

      // explicit single non-default parameter constructor (c++11)
      explicit Spec(std::string);
    };
  }
}
