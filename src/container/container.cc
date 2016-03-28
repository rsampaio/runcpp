#include "container.h"

namespace runcpp {
  namespace container {

    Container::Container(std::string cp) : container_path(cp){};

    void Container::Start() {
      fs::path config_file(fs::path(this->container_path) /
                           fs::path("config.json"));

      std::cout << "starting container: " << fs::absolute(config_file) << "\n";

      if (fs::is_regular_file(config_file)) {
        runcpp::spec::Spec spec(config_file.string());
        std::cout << "version: " << spec.version << "\n";
        std::cout << "root.path: " << spec.root.path << "\n";
        std::cout << "platform.os: " << spec.platform.os << "\n";
      }
    }

    void Container::Stop() {
      std::cout << "stoping container: " << this->id << "\n";
    }
  }
}
