#include "spec.h"

namespace runcpp {
  Spec::Spec(std::string spec_file) {
    std::ifstream spec(spec_file);
    json spec_json;

    spec >> spec_json;
    spec.close();

    this->oci_version = spec_json["ociVersion"];

    // Root
    this->root.path = spec_json["root"]["path"];
    this->root.readonly = spec_json["root"]["readonly"];

    // Platform
    this->platform.os = spec_json["platform"]["os"];
    this->platform.arch = spec_json["platform"]["arch"];

    // Processes
    this->process.env =
        spec_json["process"]["env"].get<std::vector<std::string>>();
    this->process.args =
        spec_json["process"]["args"].get<std::vector<std::string>>();
    this->process.cwd = spec_json["process"]["cwd"];

    //  user
    if (spec_json["process"]["user"]["uid"] != nullptr)
      this->process.user.uid = spec_json["process"]["user"]["uid"];
    if (spec_json["process"]["user"]["gid"] != nullptr)
      this->process.user.gid = spec_json["process"]["user"]["gid"];

    // tty
    this->process.terminal = spec_json["process"]["terminal"];

    // Mounts
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

    // Namespaces
    for (auto ns : spec_json["linux"]["namespaces"]) {
      Namespace n_ns;
      n_ns.ns_type = ns["type"];
      if (ns["path"] != nullptr)
        n_ns.path = ns["path"];
      this->spec_linux.namespaces.push_back(n_ns);
    }
  }
}
