#include "runc.h"

#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

static const char USAGE[] = R"(runcpp.

  Usage:
    runcpp start <container_dir>
    runcpp stop <container_id>
    runcpp exec [-i|--interactive] [-t|--tty] <container_id> <command>

  Options:
    --version         Show version
    -i --interactive  Exec command interactive
    -t --tty          Create a TTY
)";

int main(int argc, char *argv[]) {

  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

  std::map<std::string, docopt::value> args =
      docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "RunCpp 0.1");

  if (args["--version"]) {
    std::cout << "0.1\n";
    exit(0);
  }

  if (args["start"].asBool()) {
    runcpp::Container container(args["<container_dir>"].asString());
    container.Start();
  }

  return 0;
}
