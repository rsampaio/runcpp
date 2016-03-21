#include "container.h"

namespace runcpp {
Container::Container(std::string cp) : container_path(cp){};

void Container::Start() {
  std::cout << "starting container: "
            << "\n";
}
}
