#ifndef PARSERFACTORY_H
#define PARSERFACTORY_H

#include "kernel/simulator/Parser_if.h"

class Model;

extern "C" {
	Parser_if* genesys_createParser(Model* model, Sampler_if* sampler);
	void genesys_destroyParser(Parser_if* parser);
}

#endif /* PARSERFACTORY_H */
