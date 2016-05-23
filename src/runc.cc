#include "runc.h"
#include "container.h"

static const char USAGE[] = R"(runcpp.

  Usage:
    runcpp start [-i|--interactive] [-t|--tty] <container_dir>
    runcpp stop <container_id>
    runcpp exec [-i|--interactive] [-t|--tty] <container_id> <command>

  Options:
    --version         Show version
    -i --interactive  Exec command interactive
    -t --tty          Create a TTY
)";

int main(int argc, char *argv[]) {
  std::map<std::string, docopt::value> args =
      docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "RunCpp 0.1");

  if (args["start"].asBool()) {
    uuid_t uuid;
    char uuid_readable[37]; // uuid size 36 + \0
    int exit_status;
    std::string container_id;

    // Read spec
    runcpp::spec::Spec spec(args["<container_dir>"].asString());

    // Container
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_readable);
    container_id = std::string(uuid_readable);
    runcpp::container::Container container(container_id, spec);
    LOG(INFO) << container.id;

    // Process
    runcpp::process::Process process;
    process.args = spec.process.args;
    process.env = spec.process.env;

    int pid = fork();
    if (pid == 0) {
      container.Start(process);
    } else if (pid > 0) {
      exit_status = process.Wait();
      container.Destroy();
      exit(exit_status);
    }
  }

  return 0;
}
