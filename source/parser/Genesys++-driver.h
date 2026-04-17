#ifndef GENESYSPP_DRIVER_H
#define GENESYSPP_DRIVER_H

#include <string>
#include <map>
#include "GenesysParser.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/util/Util.h"
#include "kernel/statistics/Sampler_if.h"
// Tell Flex the lexer's prototype ...


#define YY_DECL \
  yy::genesyspp_parser::symbol_type yylex (genesyspp_driver& driver)

/* # define YY_DECL \
//  yy::genesyspp_parser::symbol_type yylex (semantic_type* yylval, location_type* yylloc, genesyspp_driver& driver)
 */
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of Calc++.

//namespace GenesysKernel {

class genesyspp_driver {
public:
	genesyspp_driver();
	genesyspp_driver(/*GenesysKernel::*/Model* model, Sampler_if* sampler, bool throws = false);
	virtual ~genesyspp_driver();
	genesyspp_driver(const genesyspp_driver& other);
	genesyspp_driver& operator=(const genesyspp_driver& other);
	genesyspp_driver(genesyspp_driver&& other) noexcept;
	genesyspp_driver& operator=(genesyspp_driver&& other) noexcept;
public:
	// Handling the scanner.
	void scan_begin_file();
	void scan_end_file();
	void scan_begin_str();
	void scan_end_str();

	// Run the parser on file F.
	// Return 0 on success.
	int parse_file(const std::string& f);
	int parse_str(const std::string& str);

	// Error handling.
	void error(const yy::location& l, const std::string& m);
	void error(const std::string& m);

	double getResult();
	void setResult(double value);

	bool getThrowsException();
	void setThrowsException(bool throws);

	void setErrorMessage(std::string message);
	std::string getErrorMessage();

	/*GenesysKernel::*/Model* getModel();
	//Sampler_if* getProbs();

	std::string getFile();
	void setFile(std::string f);

	std::string getStrToParse();
	void setStrToParse(std::string str);

public: // Sampler   
	void setSampler(Sampler_if* _sampler);
	Sampler_if* getSampler() const;

public: // trying to get infos about ModelDataElements refered in expressions (so they are not considered orphans
	void setRegisterReferedDataElements(bool value);
	bool getRegisterReferedDataElements();
	std::map<std::string, std::list<std::string>*>* getReferedDataElements();
	void clearReferedDataElements();
	void addRefered(std::pair<std::string,std::string> referedElement);

public: // SimulationResponse / SimulationControl helpers for parser semantics
	SimulationResponse* findSimulationResponse(const std::string& name) const;
	SimulationControl* findSimulationControl(const std::string& name) const;
	double getSimulationResponseValueAsDouble(const std::string& name) const;
	double getSimulationControlValueAsDouble(const std::string& name) const;
	double stringToDoubleOrWarn(const std::string& sourceType, const std::string& symbolName, const std::string& valueText) const;
	void traceWarning(const std::string& message) const;

private:
	/*GenesysKernel::*/Model* _model = nullptr;
	Sampler_if* _sampler = nullptr;
	std::map<std::string, std::list<std::string>*>* _referedDataElements = new std::map<std::string, std::list<std::string>*>(); // maps each dataelement class referenced to a list of referenced names
	bool _isRegisterReferedDataElements = false;
private:
	double result = 0;
	std::string file;
	std::string str_to_parse;
	bool throwsException = true;
	std::string errorMessage;
};
//}
#endif // ! GENESYSPP_DRIVER_H
