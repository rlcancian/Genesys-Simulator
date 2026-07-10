#ifndef BIOSBMLBRIDGE_H
#define BIOSBMLBRIDGE_H

#include <string>
#include <vector>

class Model;

struct BioSBMLImportResult {
	std::string networkName;
	unsigned int speciesImported = 0;
	unsigned int parametersImported = 0;
	unsigned int reactionsImported = 0;
	std::vector<std::string> warnings;
};

struct BioSBMLExportResult {
	std::string networkName;
	unsigned int speciesExported = 0;
	unsigned int parametersExported = 0;
	unsigned int reactionsExported = 0;
	std::vector<std::string> warnings;
};

class BioSBMLBridge {
public:
	static bool importFromString(Model* model,
	                             const std::string& sbmlText,
	                             const std::string& preferredNetworkName,
	                             BioSBMLImportResult* result,
	                             std::string& errorMessage);

	static bool exportToString(Model* model,
	                           const std::string& requestedNetworkName,
	                           std::string* sbmlText,
	                           BioSBMLExportResult* result,
	                           std::string& errorMessage);
};

#endif /* BIOSBMLBRIDGE_H */
