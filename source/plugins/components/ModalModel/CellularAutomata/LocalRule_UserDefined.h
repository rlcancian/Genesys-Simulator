#pragma once

// LocalRule_UserDefined — a cellular-automaton local rule whose transition function is supplied by
// the user as C++ source and compiled at runtime to a shared library (via the GenESyS CppCompiler),
// then loaded and invoked per cell.
// This is "Linha B" of Tema 6: arbitrary local rules defined by the user, without recompiling GenESyS.
//
// The user only writes a single C-linkage function with this signature (no GenESyS headers needed):
//
//     extern "C" long nextState(long self, const long* neighbors, int numNeighbors);
//
//   - self         : current state of the cell (a long).
//   - neighbors    : current states of the cell's neighbors, in the neighborhood's canonical order
//                    (for a 1D centered radius-1 neighborhood this is {left, right}).
//   - numNeighbors : how many neighbors were provided.
//   - return value : the cell's next state.
//
// Example (elementary rule 90, next = left XOR right):
//     extern "C" long nextState(long self, const long* n, int) { return n[0] ^ n[1]; }
//
// The function is pure integer arithmetic, so it compiles standalone (g++ -shared -fPIC), which keeps
// runtime compilation fast and portable between Linux (.so) and macOS (clang also accepts -shared).
//
// Design mirrors the existing CppForG component (plugins/components/ExternalIntegration/CppForG.cpp),
// which already does compile -> load -> dlsym with extern "C" symbols.

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"

#include <dlfcn.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// The user-provided transition function, resolved by name "nextState" from the compiled library.
// Declared at namespace scope with C linkage (as CppForG does), so dlsym finds the unmangled symbol.
extern "C" typedef long (*NextStateFunction)(long self, const long* neighbors, int numNeighbors);

class LocalRule_UserDefined : public LocalRule {
public:
	// The CppCompiler is injected (not owned): the caller controls its model, compiler command and
	// output/temp directories. Those directories must exist and be writable before build() is called.
	LocalRule_UserDefined(CellularAutomataBase* parentCellularAutomata, CppCompiler* compiler, StateSet* stateSet = nullptr)
		: LocalRule(parentCellularAutomata, stateSet) {
		this->compiler = compiler;
	}
	LocalRule_UserDefined(const LocalRule_UserDefined& orig) : LocalRule(orig) {}
	virtual ~LocalRule_UserDefined() {
		if (libraryLoaded && compiler != nullptr) {
			compiler->unloadLibrary();
		}
	}

public:
	// Compiles `userSource` (which must define `extern "C" long nextState(long, const long*, int)`)
	// into a shared library, loads it, and resolves the `nextState` symbol. Returns false and fills
	// `errorMessage` on any failure (write error, compilation error, load error, missing symbol);
	// on failure the rule stays not-ready and applyRule() leaves cells unchanged.
	bool build(const std::string& userSource, std::string& errorMessage) {
		if (compiler == nullptr) {
			errorMessage += "LocalRule_UserDefined: no CppCompiler was provided.";
			return false;
		}
		// Working directory for the generated source and library. Use the compiler's temp dir (the
		// caller points it at a writable location, e.g. the component's .temp/). The output dir is
		// cleared below so the compiled library path equals the filename we set, which the success check stats.
		std::string workDir = compiler->getTempDir();
		if (workDir.empty()) {
			workDir = "./";
		}
		if (workDir.back() != '/') {
			workDir += '/';
		}
		std::error_code dirError;
		std::filesystem::create_directories(workDir, dirError); // best-effort; ofstream check below is authoritative
		// Unique base name per build so dlopen never returns a stale cached mapping of an overwritten
		// library (a real hazard when the same rule is rebuilt or several rules coexist).
		const std::string base = "genesys_user_local_rule_" + std::to_string(buildCounter++);
		const std::string sourceFile = workDir + base + ".cpp";
		const std::string libraryFile = workDir + base + ".so";
		std::ofstream out(sourceFile.c_str());
		if (!out.good()) {
			errorMessage += "LocalRule_UserDefined: cannot write source file '" + sourceFile +
				"' (does the working directory exist and is it writable?).";
			return false;
		}
		out << userSource;
		out.close();

		compiler->setSourceFilename(sourceFile);
		compiler->setOutputFilename(libraryFile); // path equals what the compiler's success check stats
		compiler->setOutputDir("");               // so the compiled path equals libraryFile exactly

		// The generated .cpp/.so are only needed transiently: the compiler consumes the source, and the
		// loaded library stays mapped after dlopen even once its file is unlinked. Remove both on every
		// exit so they don't pile up across rebuilds (the unique names already prevent dlopen cache reuse).
		auto removeBuildFiles = [&]() {
			std::error_code removeError;
			std::filesystem::remove(sourceFile, removeError);
			std::filesystem::remove(libraryFile, removeError);
		};

		const CppCompiler::CompilationResult result = compiler->compileToDynamicLibrary();
		if (!result.success) {
			errorMessage += "LocalRule_UserDefined: compilation failed:\n" + result.compilationErrOutput;
			removeBuildFiles();
			return false;
		}
		if (libraryLoaded) {
			compiler->unloadLibrary();
			libraryLoaded = false;
		}
		if (!compiler->loadLibrary(errorMessage)) {
			removeBuildFiles();
			return false;
		}
		libraryLoaded = true;
		void* handle = compiler->getDynamicLibraryHandler();
		if (handle == nullptr) {
			errorMessage += "LocalRule_UserDefined: dynamic library handle is null after load.";
			removeBuildFiles();
			return false;
		}
		dlerror(); // clear any stale error
		ruleFunction = reinterpret_cast<NextStateFunction>(dlsym(handle, "nextState"));
		const char* symbolError = dlerror();
		if (ruleFunction == nullptr || symbolError != nullptr) {
			errorMessage += "LocalRule_UserDefined: could not resolve symbol 'nextState': " +
				std::string(symbolError != nullptr ? symbolError : "symbol is null");
			ruleFunction = nullptr;
			removeBuildFiles();
			return false;
		}
		removeBuildFiles();
		return true;
	}

	// Convenience: wraps a function BODY (statements that return a long) into a full compilable source
	// exposing the required extern "C" nextState signature. Lets a user write just the rule logic,
	// e.g. wrapBody("return neighbors[0] ^ neighbors[1];") for elementary rule 90.
	static std::string wrapBody(const std::string& body) {
		return std::string(
			"extern \"C\" long nextState(long self, const long* neighbors, int numNeighbors) {\n") +
			"\t(void) self; (void) neighbors; (void) numNeighbors;\n\t" + body + "\n}\n";
	}

	// Convenience: builds a rule given only its function BODY, wrapping it via wrapBody() into the
	// required extern "C" nextState signature. Equivalent to build(wrapBody(body), errorMessage). Use
	// for a simple body; for a full translation unit (helpers, includes, several functions) use build().
	bool buildBody(const std::string& body, std::string& errorMessage) {
		return build(wrapBody(body), errorMessage);
	}

	bool isReady() const { return ruleFunction != nullptr; }

public:
	virtual void applyRule(Cell* cell) override {
		if (ruleFunction == nullptr) {
			return; // not built yet: leave the cell unchanged
		}
		const std::vector<Cell*> neighbors = cell->getNeighbors();
		std::vector<long> neighborStates;
		neighborStates.reserve(neighbors.size());
		for (Cell* neighbor : neighbors) {
			neighborStates.emplace_back(neighbor->getCurrentState().getValue());
		}
		const long self = cell->getCurrentState().getValue();
		const long next = ruleFunction(self, neighborStates.data(), static_cast<int>(neighborStates.size()));
		cell->setNextState(State(next));
	}

private:
	CppCompiler* compiler = nullptr; // injected, not owned
	NextStateFunction ruleFunction = nullptr;
	bool libraryLoaded = false;
	inline static int buildCounter = 0; // gives each compiled library a unique name
};
