#include <iostream>
#include <string>

namespace runcpp {
    class Container {
        std::string id;
        std::string name;
        std::string container_path;
    public:
        Container(std::string);
        void Start();
        void Stop();
        void Exec(std::string cmd);
    };
}
