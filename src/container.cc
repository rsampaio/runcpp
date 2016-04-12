#include "container.h"

namespace runcpp {
  Container::Container(std::string cp) : container_path(cp){};

  void Container::Start() {
    fs::path config_file(fs::path(this->container_path) /
                         fs::path("config.json"));

    LOG(INFO) << "starting container: " << fs::absolute(config_file);

    if (fs::is_regular_file(config_file)) {
      // Load spec
      runcpp::Spec spec(config_file.string());
      runcpp::ProcessRunner proc(spec);
      proc.Start();
      int exit_status = proc.Wait();
      exit(exit_status);
    }
  }

  void Container::Stop() {
    std::cout << "stoping container: " << this->id << "\n";
  }
}
