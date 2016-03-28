#include "spec.h"

namespace runcpp {
  namespace spec {
    Spec::Spec(std::string spec_file) {
      std::ifstream spec(spec_file);
      json spec_json;

      spec >> spec_json;
      spec.close();

      this->version = spec_json["version"];

      // Root
      Root root;
      root.path = spec_json["root"]["path"];
      root.readonly = spec_json["root"]["readonly"];
      this->root = root;

      // Platform
      Platform plat;
      plat.os = spec_json["platform"]["os"];
      plat.arch = spec_json["platform"]["arch"];
      this->platform = plat;

      // Processes
      for (auto p : spec_json["processes"]) {
        Process process;
        process.env.merge(p["env"]);
        process.args.merge(p["args"]);
        process.cwd = p["cwd"];
        process.user = p["user"];
        process.tty = p["tty"];
        this->processes.push_back(process);
      }

      // Mount
      for (auto m : spec_json["mounts"]) {
        Mount mount;
        mount.destination = m["destination"];
        mount.type = m["type"];
        mount.source = m["source"];
        for (auto const &o : m["options"]) {
          mount.options.push_back(o);
        }
        this->mounts.push_back(mount);
      }
    }
  }
}
