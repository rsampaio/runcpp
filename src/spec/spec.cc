#include "spec.h"

namespace runcpp {
  namespace spec {
    Spec::Spec(std::string spec_file) {
      std::ifstream spec(spec_file);
      json spec_json;

      spec >> spec_json;
      spec.close();

      this->version = spec_json["version"];
      for (auto p : spec_json["processes"]) {
        runcpp::spec::Process process;
        process.env.merge(p["env"]);
        process.args.merge(p["args"]);
        this->processes.push_back(process);
      }
    }
  }
}
