#include "runc.h"
#include "container.h"

static const char USAGE[] = R"(runcpp.

  Usage:
    runcpp [--log=<log>] [--log-format=<log_format>] start [-b|--bundle=<bundle_dir>] [--console=<console>] [-d|--detach] [--pid-file=<pid_file>] [--no-subreaper] <container_id>
    runcpp stop <container_id>
    runcpp delete <container_id>
    runcpp init

  Options:
    -d --detach                    Detach
    -b --bundle=<bundle_dir>       Bundle directory
    --version                      Show version
    --console=<console>            Set console device
    --pid-file=<pid_file>          Set pid file
)";

int main(int argc, char *argv[]) {
  std::map<std::string, docopt::value> args =
      docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "RunCpp 0.1");
  if (args["init"].asBool()) {
    int pipefd = 0, statefd = 0;
    if (getenv("_RUNCPP_INITPIPE") != nullptr) {
      pipefd = atoi(getenv("_RUNCPP_INITPIPE"));
    }

    runcpp::container::Container::InitContainer(pipefd, statefd);
  } else if (args["start"].asBool()) {
    int exit_status;
    std::string container_id;
    std::string bundle;
    std::string pid_file;

    if (args["--pid-file"] && args["--pid-file"].isString()) {
      pid_file = args["--pid-file"].asString();
    }
    // Bundle or current dir
    if (args["--bundle"] && args["--bundle"].isStringList()) {
      bundle = args["--bundle"].asStringList().front();
    } else {
      bundle = fs::current_path().string();
    }
    runcpp::spec::Spec spec(bundle);

    // Container
    container_id = args["<container_id>"].asString();
    runcpp::container::Container container(container_id, spec);
    container.pid_file = pid_file;

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
      if (container.pid_file != "") {
        fs::remove_all(fs::path(container.pid_file));
      }
      exit(exit_status);
    }
  } else if (args["delete"].asBool()) {
    std::cout << "deleting " << args["<container_id>"].asString();
  }

  return 0;
}
