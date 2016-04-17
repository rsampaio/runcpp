#include "runc.h"
#include "container.h"

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
    uuid_t uuid;
    int exit_status;
    std::string container_id;

    uuid_generate(uuid);
    container_id = std::string(reinterpret_cast<char *>(uuid));

    // Spec
    fs::path config_file(fs::path(args["<container_dir>"].asString()) /
                         fs::path("config.json"));

    runcpp::spec::Spec spec(config_file.string());

    // Process
    runcpp::process::Process process;
    process.args = spec.process.args;
    process.env = spec.process.env;

    runcpp::container::Container container(container_id, spec);

    container.Start(process);
    exit_status = process.Wait();
    container.Destroy();
    exit(exit_status);
  }

  return 0;
}
