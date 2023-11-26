#include <unistd.h>
#include <csignal>

#include "./ParallelAndDistributedManager.h"
#include "../../kernel/simulator/Simulator.h"

int main(int argc, char** argv) {
	signal(SIGPIPE, SIG_IGN);
	ParallelAndDistributedManager* app = new ParallelAndDistributedManager();
	app->main(argc, argv);

	return 0;
}
