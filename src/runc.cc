#include "runc.h"
#include "container.h"

static const char USAGE[] = R"(runcpp.

  Usage:
    runcpp start [-b|--bundle=<bundle_dir>] [--console] [--detach] [--pid-file] [--no-subreaper] <container_id>
    runcpp stop <container_id>
    runcpp delete <container_id>
    runcpp init

  Options:
    -b --bundle=<bundle_dir>       Bundle directory
    --version                      Show version
)";

int main(int argc, char *argv[]) {
  std::map<std::string, docopt::value> args =
      docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "RunCpp 0.1");
  if (args["init"].asBool()) {
    int pipefd = 0, statefd = 0;
    LOG(INFO) << "init";
    if (getenv("_LIBCONTAINER_INITPIPE") != nullptr) {
      pipefd = atoi(getenv("_LIBCONTAINER_INITPIPE"));
    }

    if (getenv("_LIBCONTAINER_STATEDIR") != nullptr) {
      statefd = atoi(getenv("_LIBCONTAINER_STATEDIR"));
    }

    runcpp::container::Container::InitContainer(pipefd, statefd);
  } else if (args["start"].asBool()) {
    int exit_status;
    std::string container_id;
    std::string bundle;

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
  } else if (args["delete"].asBool()) {
    LOG(INFO) << "deleting " << args["<container_id>"].asString();
  }

  return 0;
}
