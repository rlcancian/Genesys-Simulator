#include "ParserFactory.h"
#include "Genesys++-driver.h"
#include "kernel/simulator/ParserDefaultImpl2.h"

extern "C" Parser_if* genesys_createParser(Model* model, Sampler_if* sampler) {
	return new ParserDefaultImpl2(model, sampler);
}

extern "C" void genesys_destroyParser(Parser_if* parser) {
	delete parser;
}
