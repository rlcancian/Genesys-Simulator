#pragma once

// LocalRule_UserDefined — a cellular-automaton local rule whose transition function is supplied by
// the user as C++ source and compiled at runtime to a shared library (via the GenESyS CppCompiler),
// then loaded and invoked per cell.
// This is "Linha B" of Tema 6: arbitrary local rules defined by the user, without recompiling GenESyS.
//
// The user writes one C-linkage function (no GenESyS headers needed). Two contracts are accepted:
//
//   (simple)   extern "C" long nextState(long self, const long* neighbors, int numNeighbors);
//   (extended) extern "C" long nextStateEx(long self, const long* neighbors, int numNeighbors,
//                                           const int* position, int numDimensions);
//
//   - self         : current state of the cell (a long).
//   - neighbors    : current states of the cell's neighbors, in the neighborhood's canonical order
//                    (for a 1D centered radius-1 neighborhood this is {left, right}).
//   - numNeighbors : how many neighbors were provided.
//   - position     : (extended only) the cell's n-dimensional coordinate, length numDimensions.
//   - numDimensions: (extended only) the lattice dimensionality.
//   - return value : the cell's next state.
//
// The extended contract lets a rule depend on the cell's position in the lattice (tema 6 §6: the
// local rule may need "a posição da célula no lattice ... a dimensão do lattice"). If the compiled
// library exports nextStateEx it is used; otherwise nextState is used. At least one must be present.
//
// Example (elementary rule 90, next = left XOR right):
//     extern "C" long nextState(long self, const long* n, int) { return n[0] ^ n[1]; }
// Example (position-dependent: a cell's next state is the parity of its first coordinate):
//     extern "C" long nextStateEx(long, const long*, int, const int* pos, int) { return pos[0] & 1; }
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
// Optional extended contract: also receives the cell's n-dimensional position (length numDimensions),
// resolved by name "nextStateEx" if the library exports it. Lets rules depend on cell coordinates.
extern "C" typedef long (*NextStateExFunction)(long self, const long* neighbors, int numNeighbors,
	const int* position, int numDimensions);

class LocalRule_UserDefined : public LocalRule {
public:
	// The CppCompiler is injected (not owned): the caller controls its model, compiler command and
	// output/temp directories. Those directories must exist and be writable before build() is called.
	LocalRule_UserDefined(CellularAutomataBase* parentCellularAutomata, CppCompiler* compiler, StateSet* stateSet = nullptr)
		: LocalRule(parentCellularAutomata, stateSet) {
		this->compiler = compiler;
	}
	// Non-copyable: it owns a loaded dynamic library and a function pointer into it. A shallow copy
	// would either alias one library across two owners (double unload) or, as the old default-bodied
	// copy did, leave the copy with compiler==nullptr/ruleFunction==nullptr (a silent no-op rule).
	LocalRule_UserDefined(const LocalRule_UserDefined&) = delete;
	LocalRule_UserDefined& operator=(const LocalRule_UserDefined&) = delete;
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
		// Prefer the extended symbol (gives the rule the cell position); fall back to the simple one.
		// A missing optional symbol is not an error, so dlerror() is cleared between lookups; at least
		// one of the two must resolve.
		dlerror();
		ruleFunctionEx = reinterpret_cast<NextStateExFunction>(dlsym(handle, "nextStateEx"));
		dlerror();
		ruleFunction = reinterpret_cast<NextStateFunction>(dlsym(handle, "nextState"));
		dlerror();
		if (ruleFunctionEx == nullptr && ruleFunction == nullptr) {
			errorMessage += "LocalRule_UserDefined: could not resolve symbol 'nextState' or 'nextStateEx' "
				"(the user source must define one of them).";
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

	bool isReady() const { return ruleFunction != nullptr || ruleFunctionEx != nullptr; }

public:
	virtual void applyRule(Cell* cell) override {
		if (ruleFunction == nullptr && ruleFunctionEx == nullptr) {
			return; // not built yet: leave the cell unchanged
		}
		const std::vector<Cell*> neighbors = cell->getNeighbors();
		// Reuse a member buffer across cells/steps instead of allocating a fresh vector per cell per
		// step (clear() keeps the capacity), which matters on large lattices over many generations.
		neighborStatesBuffer.clear();
		neighborStatesBuffer.reserve(neighbors.size());
		for (Cell* neighbor : neighbors) {
			neighborStatesBuffer.emplace_back(neighbor->getCurrentState().getValue());
		}
		const long self = cell->getCurrentState().getValue();
		const int numNeighbors = static_cast<int>(neighborStatesBuffer.size());
		long next;
		if (ruleFunctionEx != nullptr) {
			// Extended contract: also hand the rule the cell's n-dimensional position so it can be
			// position-dependent (tema 6 §6). getPosition() is populated by Lattice::init().
			const std::vector<int> position = cell->getPosition();
			next = ruleFunctionEx(self, neighborStatesBuffer.data(), numNeighbors,
				position.data(), static_cast<int>(position.size()));
		} else {
			next = ruleFunction(self, neighborStatesBuffer.data(), numNeighbors);
		}
		cell->setNextState(State(next));
	}

private:
	CppCompiler* compiler = nullptr; // injected, not owned
	NextStateFunction ruleFunction = nullptr;     // simple contract symbol, or null if the rule uses Ex
	NextStateExFunction ruleFunctionEx = nullptr; // extended contract symbol (preferred when present)
	bool libraryLoaded = false;
	std::vector<long> neighborStatesBuffer; // reused across applyRule() calls to avoid per-cell allocation
	inline static int buildCounter = 0; // gives each compiled library a unique name
};
