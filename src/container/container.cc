#include "container.h"

namespace runcpp {
  namespace container {

    Container::Container(std::string cp) : container_path(cp){};

    void Container::Start() {
      fs::path cpath = this->container_path;
      fs::path config_file = cpath / "config.json";

      std::cout << "starting container: " << fs::absolute(cpath) << "\n";

      if (fs::is_regular_file(config_file)) {
        runcpp::spec::Spec spec(config_file.string());
        std::cout << spec.version << "\n";
      }
    }

    void Container::Stop() {
      std::cout << "stoping container: " << this->id << "\n";
    }
  }
}
