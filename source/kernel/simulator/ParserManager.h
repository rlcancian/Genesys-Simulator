/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ParserManager.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 20:34
 */

#ifndef PARSERMANAGER_H
#define PARSERMANAGER_H

#include "ParserChangesInformation.h"

//namespace GenesysKernel {

class ParserManager {
public:

	/**
	 * @brief Paths associated with a generated parser artifact set.
	 */
	struct NewParser {
		std::string bisonFilename;
		std::string flexFilename;
		std::string compiledParserFilename;
	};

	/**
	 * @brief Result bundle for parser generation requests.
	 */
	struct GenerateNewParserResult {
		bool result = false;
		std::string bisonMessages;
		std::string lexMessages;
		std::string compilationMessages;
		NewParser newParser;
	};
public:
	ParserManager();
	virtual ~ParserManager() = default;
public:
	/*!
	 * \brief generateNewParser
	 * \param changes
	 * \return
	 */
	// @ToDo: (importante): Implement parser generation workflow and define the expected filesystem/toolchain contract.
	ParserManager::GenerateNewParserResult generateNewParser(ParserChangesInformation* changes);
	/*!
	 * \brief connectNewParser
	 * \param newParser
	 * \return
	 */
	// @ToDo: (importante): Implement parser loading/activation and define ownership/lifetime rules for connected parsers.
	bool connectNewParser(ParserManager::NewParser newParser);
private:
};
//namespace\\}
#endif /* PARSERMANAGER_H */
