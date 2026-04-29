#include "plugins/data/BiochemicalSimulation/BioSBMLBridge.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

namespace {

std::string trim(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\n\r\f\v");
	if (first == std::string::npos) {
		return "";
	}
	const size_t last = text.find_last_not_of(" \t\n\r\f\v");
	return text.substr(first, last - first + 1);
}

std::string toLower(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return text;
}

std::string decodeXmlEntities(std::string value) {
	const std::vector<std::pair<std::string, std::string>> entities = {
		{"&quot;", "\""},
		{"&apos;", "'"},
		{"&lt;", "<"},
		{"&gt;", ">"},
		{"&amp;", "&"}
	};
	for (const auto& [entity, replacement] : entities) {
		size_t position = 0;
		while ((position = value.find(entity, position)) != std::string::npos) {
			value.replace(position, entity.size(), replacement);
			position += replacement.size();
		}
	}
	return value;
}

std::string encodeXmlEntities(const std::string& value) {
	std::string escaped;
	escaped.reserve(value.size());
	for (const char ch : value) {
		switch (ch) {
			case '&':
				escaped += "&amp;";
				break;
			case '<':
				escaped += "&lt;";
				break;
			case '>':
				escaped += "&gt;";
				break;
			case '\"':
				escaped += "&quot;";
				break;
			case '\'':
				escaped += "&apos;";
				break;
			default:
				escaped += ch;
				break;
		}
	}
	return escaped;
}

std::string collapseWhitespace(const std::string& text) {
	std::ostringstream out;
	bool previousWhitespace = false;
	for (unsigned char ch : text) {
		if (std::isspace(ch)) {
			if (!previousWhitespace) {
				out << ' ';
				previousWhitespace = true;
			}
		} else {
			out << static_cast<char>(ch);
			previousWhitespace = false;
		}
	}
	return trim(out.str());
}

bool tryParseDouble(const std::string& text, double* value) {
	if (value == nullptr) {
		return false;
	}
	try {
		const std::string normalized = trim(text);
		if (normalized.empty()) {
			return false;
		}
		size_t consumed = 0;
		const double parsed = std::stod(normalized, &consumed);
		if (consumed != normalized.size()) {
			return false;
		}
		*value = parsed;
		return true;
	} catch (...) {
		return false;
	}
}

bool parseBool(const std::string& text, bool defaultValue) {
	const std::string normalized = toLower(trim(text));
	if (normalized == "1" || normalized == "true" || normalized == "yes") {
		return true;
	}
	if (normalized == "0" || normalized == "false" || normalized == "no") {
		return false;
	}
	return defaultValue;
}

bool isIdentifierLike(const std::string& symbol) {
	if (symbol.empty()) {
		return false;
	}
	if (!(std::isalpha(static_cast<unsigned char>(symbol.front())) || symbol.front() == '_')) {
		return false;
	}
	for (char ch : symbol) {
		if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
			return false;
		}
	}
	return true;
}

std::string sanitizeIdentifier(const std::string& text, const std::string& fallback) {
	std::string sanitized;
	sanitized.reserve(text.size());
	for (char ch : text) {
		if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
			sanitized += ch;
		} else {
			sanitized += '_';
		}
	}
	sanitized = trim(sanitized);
	if (sanitized.empty()) {
		sanitized = fallback;
	}
	if (sanitized.empty()) {
		sanitized = "item";
	}
	if (!(std::isalpha(static_cast<unsigned char>(sanitized.front())) || sanitized.front() == '_')) {
		sanitized.insert(sanitized.begin(), '_');
	}
	return sanitized;
}

std::map<std::string, std::string> parseAttributes(const std::string& attributesText) {
	std::map<std::string, std::string> attributes;
	const std::regex attributePattern("([A-Za-z_][A-Za-z0-9_.:-]*)\\s*=\\s*(\"([^\"]*)\"|'([^']*)')");
	auto begin = std::sregex_iterator(attributesText.begin(), attributesText.end(), attributePattern);
	auto end = std::sregex_iterator();
	for (auto it = begin; it != end; ++it) {
		const std::smatch& match = *it;
		std::string value;
		if (match[3].matched) {
			value = match[3].str();
		} else if (match[4].matched) {
			value = match[4].str();
		}
		attributes[toLower(match[1].str())] = decodeXmlEntities(value);
	}
	return attributes;
}

std::string attributeValue(const std::map<std::string, std::string>& attributes, const std::string& key) {
	auto it = attributes.find(toLower(key));
	if (it == attributes.end()) {
		return "";
	}
	return it->second;
}

bool extractFirstSection(const std::string& text, const std::string& tagName, std::string* sectionBody) {
	if (sectionBody == nullptr) {
		return false;
	}
	const std::regex pattern("<" + tagName + R"(\b[^>]*>([\s\S]*?)</)" + tagName + ">", std::regex::icase);
	std::smatch match;
	if (!std::regex_search(text, match, pattern)) {
		return false;
	}
	*sectionBody = match[1].str();
	return true;
}

bool extractFirstSectionWithAttributes(const std::string& text,
                                      const std::string& tagName,
                                      std::string* attributesText,
                                      std::string* sectionBody) {
	if (attributesText == nullptr || sectionBody == nullptr) {
		return false;
	}
	const std::regex pattern("<" + tagName + R"(\b([^>]*)>([\s\S]*?)</)" + tagName + ">", std::regex::icase);
	std::smatch match;
	if (!std::regex_search(text, match, pattern)) {
		return false;
	}
	*attributesText = match[1].str();
	*sectionBody = match[2].str();
	return true;
}

std::string regexEscape(const std::string& text) {
	std::string escaped;
	escaped.reserve(text.size() * 2);
	for (char ch : text) {
		switch (ch) {
			case '\\':
			case '^':
			case '$':
			case '.':
			case '|':
			case '?':
			case '*':
			case '+':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
				escaped += '\\';
				escaped += ch;
				break;
			default:
				escaped += ch;
				break;
		}
	}
	return escaped;
}

std::string mapExpressionSymbols(
		const std::string& expression,
		const std::map<std::string, std::string>& symbolMap) {
	std::string mapped = expression;
	std::vector<std::pair<std::string, std::string>> replacements(symbolMap.begin(), symbolMap.end());
	std::sort(replacements.begin(), replacements.end(), [](const auto& left, const auto& right) {
		return left.first.size() > right.first.size();
	});
	for (const auto& [from, to] : replacements) {
		if (!isIdentifierLike(from) || !isIdentifierLike(to)) {
			continue;
		}
		const std::regex pattern("\\b" + regexEscape(from) + "\\b");
		mapped = std::regex_replace(mapped, pattern, to);
	}
	return mapped;
}

std::string extractMathText(const std::string& kineticLawBody) {
	std::string mathBody;
	if (!extractFirstSection(kineticLawBody, "math", &mathBody)) {
		return "";
	}
	const std::regex tagPattern(R"(<[^>]+>)");
	const std::string noTags = std::regex_replace(mathBody, tagPattern, " ");
	return collapseWhitespace(decodeXmlEntities(noTags));
}

std::string makeUniqueDataDefinitionName(Model* model,
                                         const std::string& typeName,
                                         const std::string& preferredName,
                                         std::set<std::string>* reservedNames) {
	std::string baseName = sanitizeIdentifier(preferredName, "item");
	std::string candidate = baseName;
	unsigned int suffix = 1;
	auto isTaken = [&]() {
		const bool reserved = reservedNames != nullptr && reservedNames->find(candidate) != reservedNames->end();
		const bool existing = model != nullptr && model->getDataManager() != nullptr &&
		                      model->getDataManager()->getDataDefinition(typeName, candidate) != nullptr;
		return reserved || existing;
	};
	while (isTaken()) {
		candidate = baseName + "_" + std::to_string(suffix++);
	}
	if (reservedNames != nullptr) {
		reservedNames->insert(candidate);
	}
	return candidate;
}

std::string formatDouble(double value) {
	std::ostringstream out;
	out << std::setprecision(15) << value;
	return out.str();
}

std::string uniqueSymbolId(const std::string& preferredId,
                           const std::string& fallbackPrefix,
                           std::set<std::string>* usedIds) {
	std::string base = sanitizeIdentifier(preferredId, fallbackPrefix);
	std::string candidate = base;
	unsigned int suffix = 1;
	while (usedIds != nullptr && usedIds->find(candidate) != usedIds->end()) {
		candidate = base + "_" + std::to_string(suffix++);
	}
	if (usedIds != nullptr) {
		usedIds->insert(candidate);
	}
	return candidate;
}

bool collectNetworkSpecies(Model* model, BioNetwork* network, std::vector<BioSpecies*>* species, std::string& errorMessage) {
	if (model == nullptr || model->getDataManager() == nullptr || network == nullptr || species == nullptr) {
		errorMessage = "Invalid arguments while collecting BioSpecies for SBML export.";
		return false;
	}
	species->clear();
	if (network->getSpeciesNames().empty()) {
		List<ModelDataDefinition*>* list = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());
		for (ModelDataDefinition* definition : *list->list()) {
			auto* bioSpecies = dynamic_cast<BioSpecies*>(definition);
			if (bioSpecies != nullptr) {
				species->push_back(bioSpecies);
			}
		}
	} else {
		for (const std::string& speciesName : network->getSpeciesNames()) {
			auto* bioSpecies = dynamic_cast<BioSpecies*>(
					model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName));
			if (bioSpecies == nullptr) {
				errorMessage += "BioNetwork \"" + network->getName() + "\" references missing BioSpecies \"" + speciesName + "\". ";
				continue;
			}
			species->push_back(bioSpecies);
		}
	}
	if (species->empty()) {
		errorMessage += "BioNetwork \"" + network->getName() + "\" has no BioSpecies for SBML export.";
		return false;
	}
	return true;
}

bool collectNetworkReactions(Model* model, BioNetwork* network, std::vector<BioReaction*>* reactions, std::string& errorMessage) {
	if (model == nullptr || model->getDataManager() == nullptr || network == nullptr || reactions == nullptr) {
		errorMessage = "Invalid arguments while collecting BioReactions for SBML export.";
		return false;
	}
	reactions->clear();
	if (network->getReactionNames().empty()) {
		List<ModelDataDefinition*>* list = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioReaction>());
		for (ModelDataDefinition* definition : *list->list()) {
			auto* reaction = dynamic_cast<BioReaction*>(definition);
			if (reaction != nullptr) {
				reactions->push_back(reaction);
			}
		}
	} else {
		for (const std::string& reactionName : network->getReactionNames()) {
			auto* reaction = dynamic_cast<BioReaction*>(
					model->getDataManager()->getDataDefinition(Util::TypeOf<BioReaction>(), reactionName));
			if (reaction == nullptr) {
				errorMessage += "BioNetwork \"" + network->getName() + "\" references missing BioReaction \"" + reactionName + "\". ";
				continue;
			}
			reactions->push_back(reaction);
		}
	}
	if (reactions->empty()) {
		errorMessage += "BioNetwork \"" + network->getName() + "\" has no BioReactions for SBML export.";
		return false;
	}
	return true;
}

std::string reactionMassActionExpression(const BioReaction* reaction,
                                         const std::map<std::string, std::string>& speciesToSbmlId,
                                         const std::map<std::string, std::string>& parameterToSbmlId) {
	std::string rateTerm;
	if (!reaction->getRateConstantParameterName().empty()) {
		auto parameterIt = parameterToSbmlId.find(reaction->getRateConstantParameterName());
		rateTerm = parameterIt != parameterToSbmlId.end()
		           ? parameterIt->second
		           : sanitizeIdentifier(reaction->getRateConstantParameterName(), "k");
	} else {
		rateTerm = formatDouble(reaction->resolveRateConstant());
	}
	std::vector<std::string> factors;
	for (const BioReaction::StoichiometricTerm& term : reaction->getReactants()) {
		auto speciesIt = speciesToSbmlId.find(term.speciesName);
		if (speciesIt == speciesToSbmlId.end()) {
			continue;
		}
		if (std::abs(term.stoichiometry - 1.0) < 1e-12) {
			factors.push_back(speciesIt->second);
		} else {
			factors.push_back(speciesIt->second + "^" + formatDouble(term.stoichiometry));
		}
	}
	if (factors.empty()) {
		return rateTerm;
	}
	std::ostringstream out;
	out << rateTerm;
	for (const std::string& factor : factors) {
		out << " * " << factor;
	}
	return out.str();
}

} // namespace

bool BioSBMLBridge::importFromString(Model* model,
                                     const std::string& sbmlText,
                                     const std::string& preferredNetworkName,
                                     BioSBMLImportResult* result,
                                     std::string& errorMessage) {
	errorMessage.clear();
	if (result != nullptr) {
		*result = BioSBMLImportResult{};
	}
	if (model == nullptr || model->getDataManager() == nullptr) {
		errorMessage = "SBML import requires a valid model.";
		return false;
	}
	if (trim(sbmlText).empty()) {
		errorMessage = "SBML import received an empty document.";
		return false;
	}

	const std::regex modelPattern(R"(<model\b([^>]*)>([\s\S]*?)</model>)", std::regex::icase);
	std::smatch modelMatch;
	if (!std::regex_search(sbmlText, modelMatch, modelPattern)) {
		errorMessage = "SBML import could not locate a <model> element.";
		return false;
	}

	const std::map<std::string, std::string> modelAttributes = parseAttributes(modelMatch[1].str());
	const std::string modelBody = modelMatch[2].str();

	std::vector<std::string> importedSpeciesNames;
	std::vector<std::string> importedReactionNames;
	std::set<std::string> reservedSpeciesNames;
	std::set<std::string> reservedParameterNames;
	std::set<std::string> reservedReactionNames;
	std::map<std::string, std::string> symbolMap;

	std::string listOfSpecies;
	if (extractFirstSection(modelBody, "listOfSpecies", &listOfSpecies)) {
		const std::regex speciesPattern(R"(<species\b([^>]*)/?>)", std::regex::icase);
		auto begin = std::sregex_iterator(listOfSpecies.begin(), listOfSpecies.end(), speciesPattern);
		auto end = std::sregex_iterator();
		unsigned int anonymousSpecies = 1;
		for (auto it = begin; it != end; ++it) {
			const std::map<std::string, std::string> attributes = parseAttributes((*it)[1].str());
			std::string sbmlId = trim(attributeValue(attributes, "id"));
			const std::string sbmlName = trim(attributeValue(attributes, "name"));
			if (sbmlId.empty()) {
				sbmlId = sanitizeIdentifier(sbmlName, "S" + std::to_string(anonymousSpecies++));
			}
			const std::string preferredName = !sbmlName.empty() ? sbmlName : sbmlId;
			const std::string nativeName = makeUniqueDataDefinitionName(model, Util::TypeOf<BioSpecies>(), preferredName, &reservedSpeciesNames);

			auto* species = new BioSpecies(model, nativeName);
			double initialAmount = 0.0;
			if (!tryParseDouble(attributeValue(attributes, "initialamount"), &initialAmount)) {
				tryParseDouble(attributeValue(attributes, "initialconcentration"), &initialAmount);
			}
			species->setInitialAmount(initialAmount);
			species->setAmount(initialAmount);
			species->setConstant(parseBool(attributeValue(attributes, "constant"), false));
			species->setBoundaryCondition(parseBool(attributeValue(attributes, "boundarycondition"), false));
			const std::string unit = attributeValue(attributes, "substanceunits");
			if (!unit.empty()) {
				species->setUnit(unit);
			}
			importedSpeciesNames.push_back(nativeName);
			symbolMap[sbmlId] = nativeName;
			if (!sbmlName.empty()) {
				symbolMap[sbmlName] = nativeName;
			}
			if (result != nullptr) {
				result->speciesImported++;
				if (nativeName != preferredName) {
					result->warnings.push_back("BioSpecies \"" + preferredName + "\" imported as \"" + nativeName + "\" due to naming conflict.");
				}
			}
		}
	}

	std::string listOfParameters;
	if (extractFirstSection(modelBody, "listOfParameters", &listOfParameters)) {
		const std::regex parameterPattern(R"(<parameter\b([^>]*)/?>)", std::regex::icase);
		auto begin = std::sregex_iterator(listOfParameters.begin(), listOfParameters.end(), parameterPattern);
		auto end = std::sregex_iterator();
		unsigned int anonymousParameter = 1;
		for (auto it = begin; it != end; ++it) {
			const std::map<std::string, std::string> attributes = parseAttributes((*it)[1].str());
			std::string sbmlId = trim(attributeValue(attributes, "id"));
			const std::string sbmlName = trim(attributeValue(attributes, "name"));
			if (sbmlId.empty()) {
				sbmlId = sanitizeIdentifier(sbmlName, "k" + std::to_string(anonymousParameter++));
			}
			const std::string preferredName = !sbmlName.empty() ? sbmlName : sbmlId;
			const std::string nativeName = makeUniqueDataDefinitionName(model, Util::TypeOf<BioParameter>(), preferredName, &reservedParameterNames);

			auto* parameter = new BioParameter(model, nativeName);
			double value = 0.0;
			if (tryParseDouble(attributeValue(attributes, "value"), &value)) {
				parameter->setValue(value);
			}
			const std::string unit = attributeValue(attributes, "units");
			if (!unit.empty()) {
				parameter->setUnit(unit);
			}
			symbolMap[sbmlId] = nativeName;
			if (!sbmlName.empty()) {
				symbolMap[sbmlName] = nativeName;
			}
			if (result != nullptr) {
				result->parametersImported++;
				if (nativeName != preferredName) {
					result->warnings.push_back("BioParameter \"" + preferredName + "\" imported as \"" + nativeName + "\" due to naming conflict.");
				}
			}
		}
	}

	std::string listOfReactions;
	if (extractFirstSection(modelBody, "listOfReactions", &listOfReactions)) {
		const std::regex reactionPattern(R"(<reaction\b([^>]*)>([\s\S]*?)</reaction>)", std::regex::icase);
		auto begin = std::sregex_iterator(listOfReactions.begin(), listOfReactions.end(), reactionPattern);
		auto end = std::sregex_iterator();
		unsigned int anonymousReaction = 1;
		for (auto it = begin; it != end; ++it) {
			const std::map<std::string, std::string> reactionAttributes = parseAttributes((*it)[1].str());
			const std::string reactionBody = (*it)[2].str();
			std::string reactionId = trim(attributeValue(reactionAttributes, "id"));
			const std::string reactionDisplayName = trim(attributeValue(reactionAttributes, "name"));
			if (reactionId.empty()) {
				reactionId = sanitizeIdentifier(reactionDisplayName, "R" + std::to_string(anonymousReaction++));
			}
			const std::string preferredName = !reactionDisplayName.empty() ? reactionDisplayName : reactionId;
			const std::string reactionName = makeUniqueDataDefinitionName(model, Util::TypeOf<BioReaction>(), preferredName, &reservedReactionNames);

			auto* reaction = new BioReaction(model, reactionName);
			reaction->setReversible(parseBool(attributeValue(reactionAttributes, "reversible"), false));

			std::string reactantsBody;
			if (extractFirstSection(reactionBody, "listOfReactants", &reactantsBody)) {
				const std::regex refPattern(R"(<speciesReference\b([^>]*)/?>)", std::regex::icase);
				auto refBegin = std::sregex_iterator(reactantsBody.begin(), reactantsBody.end(), refPattern);
				auto refEnd = std::sregex_iterator();
				for (auto refIt = refBegin; refIt != refEnd; ++refIt) {
					const auto attributes = parseAttributes((*refIt)[1].str());
					const std::string speciesSymbol = attributeValue(attributes, "species");
					auto mappedSpecies = symbolMap.find(speciesSymbol);
					if (mappedSpecies == symbolMap.end()) {
						if (result != nullptr) {
							result->warnings.push_back("BioReaction \"" + reactionName + "\" ignored missing reactant species \"" + speciesSymbol + "\".");
						}
						continue;
					}
					double stoichiometry = 1.0;
					tryParseDouble(attributeValue(attributes, "stoichiometry"), &stoichiometry);
					reaction->addReactant(mappedSpecies->second, stoichiometry);
				}
			}

			std::string productsBody;
			if (extractFirstSection(reactionBody, "listOfProducts", &productsBody)) {
				const std::regex refPattern(R"(<speciesReference\b([^>]*)/?>)", std::regex::icase);
				auto refBegin = std::sregex_iterator(productsBody.begin(), productsBody.end(), refPattern);
				auto refEnd = std::sregex_iterator();
				for (auto refIt = refBegin; refIt != refEnd; ++refIt) {
					const auto attributes = parseAttributes((*refIt)[1].str());
					const std::string speciesSymbol = attributeValue(attributes, "species");
					auto mappedSpecies = symbolMap.find(speciesSymbol);
					if (mappedSpecies == symbolMap.end()) {
						if (result != nullptr) {
							result->warnings.push_back("BioReaction \"" + reactionName + "\" ignored missing product species \"" + speciesSymbol + "\".");
						}
						continue;
					}
					double stoichiometry = 1.0;
					tryParseDouble(attributeValue(attributes, "stoichiometry"), &stoichiometry);
					reaction->addProduct(mappedSpecies->second, stoichiometry);
				}
			}

			std::string modifiersBody;
			if (extractFirstSection(reactionBody, "listOfModifiers", &modifiersBody)) {
				const std::regex modifierPattern(R"(<modifierSpeciesReference\b([^>]*)/?>)", std::regex::icase);
				auto modBegin = std::sregex_iterator(modifiersBody.begin(), modifiersBody.end(), modifierPattern);
				auto modEnd = std::sregex_iterator();
				for (auto modIt = modBegin; modIt != modEnd; ++modIt) {
					const auto attributes = parseAttributes((*modIt)[1].str());
					const std::string speciesSymbol = attributeValue(attributes, "species");
					auto mappedSpecies = symbolMap.find(speciesSymbol);
					if (mappedSpecies == symbolMap.end()) {
						if (result != nullptr) {
							result->warnings.push_back("BioReaction \"" + reactionName + "\" ignored missing modifier species \"" + speciesSymbol + "\".");
						}
						continue;
					}
					reaction->addModifier(mappedSpecies->second);
				}
			}

			std::string kineticLawAttributesText;
			std::string kineticLawBody;
			std::map<std::string, std::string> kineticLawAttributes;
			std::string kineticExpression;
			if (extractFirstSectionWithAttributes(reactionBody, "kineticLaw", &kineticLawAttributesText, &kineticLawBody)) {
				kineticLawAttributes = parseAttributes(kineticLawAttributesText);
				kineticExpression = trim(attributeValue(kineticLawAttributes, "formula"));
				if (kineticExpression.empty()) {
					kineticExpression = extractMathText(kineticLawBody);
				}

				const std::regex localParameterPattern(R"(<parameter\b([^>]*)/?>)", std::regex::icase);
				auto localBegin = std::sregex_iterator(kineticLawBody.begin(), kineticLawBody.end(), localParameterPattern);
				auto localEnd = std::sregex_iterator();
				for (auto localIt = localBegin; localIt != localEnd; ++localIt) {
					const auto attributes = parseAttributes((*localIt)[1].str());
					std::string localId = trim(attributeValue(attributes, "id"));
					if (localId.empty()) {
						localId = trim(attributeValue(attributes, "name"));
					}
					if (localId.empty()) {
						continue;
					}
					const std::string parameterName = makeUniqueDataDefinitionName(
						model,
						Util::TypeOf<BioParameter>(),
						reactionName + "__" + localId,
						&reservedParameterNames);
					auto* localParameter = new BioParameter(model, parameterName);
					double localValue = 0.0;
					tryParseDouble(attributeValue(attributes, "value"), &localValue);
					localParameter->setValue(localValue);
					const std::string localUnits = attributeValue(attributes, "units");
					if (!localUnits.empty()) {
						localParameter->setUnit(localUnits);
					}
					symbolMap[localId] = parameterName;
					if (result != nullptr) {
						result->parametersImported++;
						result->warnings.push_back("SBML local parameter \"" + localId + "\" promoted to BioParameter \"" + parameterName + "\".");
					}
				}
			}

			if (kineticExpression.empty()) {
				kineticExpression = trim(attributeValue(reactionAttributes, "genesyskineticlawexpression"));
			}
			if (!kineticExpression.empty()) {
				reaction->setKineticLawExpression(mapExpressionSymbols(kineticExpression, symbolMap));
			}

			std::string rateConstantParameter = attributeValue(reactionAttributes, "genesysrateconstantparameter");
			if (rateConstantParameter.empty()) {
				rateConstantParameter = attributeValue(kineticLawAttributes, "genesysrateconstantparameter");
			}
			if (!rateConstantParameter.empty()) {
				rateConstantParameter = mapExpressionSymbols(rateConstantParameter, symbolMap);
				reaction->setRateConstantParameterName(rateConstantParameter);
			}
			double rateConstant = 0.0;
			if (tryParseDouble(attributeValue(reactionAttributes, "genesysrateconstant"), &rateConstant) ||
			    tryParseDouble(attributeValue(kineticLawAttributes, "genesysrateconstant"), &rateConstant)) {
				reaction->setRateConstant(rateConstant);
			}

			if (reaction->isReversible()) {
				std::string reverseExpression = attributeValue(reactionAttributes, "genesysreversekineticlawexpression");
				if (!reverseExpression.empty()) {
					reaction->setReverseKineticLawExpression(mapExpressionSymbols(reverseExpression, symbolMap));
				}

				std::string reverseParameter = attributeValue(reactionAttributes, "genesysreverserateconstantparameter");
				if (!reverseParameter.empty()) {
					reverseParameter = mapExpressionSymbols(reverseParameter, symbolMap);
					reaction->setReverseRateConstantParameterName(reverseParameter);
				}
				double reverseRate = 0.0;
				if (tryParseDouble(attributeValue(reactionAttributes, "genesysreverserateconstant"), &reverseRate)) {
					reaction->setReverseRateConstant(reverseRate);
				}
			}

			if (reaction->getReactants().empty() && reaction->getProducts().empty()) {
				if (result != nullptr) {
					result->warnings.push_back("BioReaction \"" + reactionName + "\" ignored because it has no valid reactants/products after mapping.");
				}
				delete reaction;
				continue;
			}
			importedReactionNames.push_back(reactionName);
			if (result != nullptr) {
				result->reactionsImported++;
				if (reactionName != preferredName) {
					result->warnings.push_back("BioReaction \"" + preferredName + "\" imported as \"" + reactionName + "\" due to naming conflict.");
				}
			}
		}
	}

	std::string modelName = trim(attributeValue(modelAttributes, "name"));
	if (modelName.empty()) {
		modelName = trim(attributeValue(modelAttributes, "id"));
	}
	const std::string preferredNetwork = !trim(preferredNetworkName).empty() ? preferredNetworkName :
	                                   (!modelName.empty() ? modelName : "ImportedBioNetwork");
	const std::string networkName = makeUniqueDataDefinitionName(model, Util::TypeOf<BioNetwork>(), preferredNetwork, nullptr);
	auto* network = new BioNetwork(model, networkName);
	for (const std::string& speciesName : importedSpeciesNames) {
		network->addSpecies(speciesName);
	}
	for (const std::string& reactionName : importedReactionNames) {
		network->addReaction(reactionName);
	}
	double startTime = network->getStartTime();
	double stopTime = network->getStopTime();
	double stepSize = network->getStepSize();
	tryParseDouble(attributeValue(modelAttributes, "genesysstarttime"), &startTime);
	tryParseDouble(attributeValue(modelAttributes, "genesysstoptime"), &stopTime);
	tryParseDouble(attributeValue(modelAttributes, "genesysstepsize"), &stepSize);
	if (!tryParseDouble(attributeValue(modelAttributes, "starttime"), &startTime)) {
		// No-op, fall back to existing value.
	}
	if (!tryParseDouble(attributeValue(modelAttributes, "stoptime"), &stopTime)) {
		// No-op, fall back to existing value.
	}
	if (!tryParseDouble(attributeValue(modelAttributes, "stepsize"), &stepSize)) {
		// No-op, fall back to existing value.
	}
	network->setStartTime(startTime);
	network->setStopTime(stopTime);
	network->setStepSize(stepSize > 0.0 ? stepSize : network->getStepSize());

	if (result != nullptr) {
		result->networkName = networkName;
	}
	return true;
}

bool BioSBMLBridge::exportToString(Model* model,
                                   const std::string& requestedNetworkName,
                                   std::string* sbmlText,
                                   BioSBMLExportResult* result,
                                   std::string& errorMessage) {
	errorMessage.clear();
	if (result != nullptr) {
		*result = BioSBMLExportResult{};
	}
	if (sbmlText == nullptr) {
		errorMessage = "SBML export requires a valid output string target.";
		return false;
	}
	*sbmlText = "";
	if (model == nullptr || model->getDataManager() == nullptr) {
		errorMessage = "SBML export requires a valid model.";
		return false;
	}

	BioNetwork* network = nullptr;
	if (!trim(requestedNetworkName).empty()) {
		network = dynamic_cast<BioNetwork*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), requestedNetworkName));
		if (network == nullptr) {
			errorMessage = "SBML export could not find BioNetwork \"" + requestedNetworkName + "\".";
			return false;
		}
	} else {
		List<ModelDataDefinition*>* networks = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
		if (networks == nullptr || networks->size() == 0) {
			errorMessage = "SBML export requires at least one BioNetwork in the model.";
			return false;
		}
		network = dynamic_cast<BioNetwork*>(networks->front());
	}

	if (network == nullptr) {
		errorMessage = "SBML export received an invalid BioNetwork.";
		return false;
	}

	std::vector<BioSpecies*> species;
	std::vector<BioReaction*> reactions;
	if (!collectNetworkSpecies(model, network, &species, errorMessage)) {
		return false;
	}
	if (!collectNetworkReactions(model, network, &reactions, errorMessage)) {
		return false;
	}

	std::vector<BioParameter*> parameters;
	List<ModelDataDefinition*>* parameterDefinitions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioParameter>());
	for (ModelDataDefinition* definition : *parameterDefinitions->list()) {
		auto* parameter = dynamic_cast<BioParameter*>(definition);
		if (parameter != nullptr) {
			parameters.push_back(parameter);
		}
	}

	std::set<std::string> usedSpeciesIds;
	std::set<std::string> usedParameterIds;
	std::set<std::string> usedReactionIds;
	std::map<std::string, std::string> speciesToSbmlId;
	std::map<std::string, std::string> parameterToSbmlId;
	std::map<std::string, std::string> reactionToSbmlId;

	for (BioSpecies* bioSpecies : species) {
		speciesToSbmlId[bioSpecies->getName()] = uniqueSymbolId(bioSpecies->getName(), "S", &usedSpeciesIds);
	}
	for (BioParameter* bioParameter : parameters) {
		parameterToSbmlId[bioParameter->getName()] = uniqueSymbolId(bioParameter->getName(), "k", &usedParameterIds);
	}
	for (BioReaction* bioReaction : reactions) {
		reactionToSbmlId[bioReaction->getName()] = uniqueSymbolId(bioReaction->getName(), "R", &usedReactionIds);
	}

	std::map<std::string, std::string> nativeToSbmlSymbols;
	for (const auto& [name, sbmlId] : speciesToSbmlId) {
		nativeToSbmlSymbols[name] = sbmlId;
	}
	for (const auto& [name, sbmlId] : parameterToSbmlId) {
		nativeToSbmlSymbols[name] = sbmlId;
	}

	std::ostringstream out;
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	out << "<sbml level=\"3\" version=\"2\" xmlns=\"http://www.sbml.org/sbml/level3/version2/core\">\n";
	out << "  <model id=\"" << encodeXmlEntities(uniqueSymbolId(network->getName(), "BioNetwork", nullptr)) << "\""
	    << " name=\"" << encodeXmlEntities(network->getName()) << "\""
	    << " genesysStartTime=\"" << encodeXmlEntities(formatDouble(network->getStartTime())) << "\""
	    << " genesysStopTime=\"" << encodeXmlEntities(formatDouble(network->getStopTime())) << "\""
	    << " genesysStepSize=\"" << encodeXmlEntities(formatDouble(network->getStepSize())) << "\""
	    << ">\n";

	out << "    <listOfSpecies>\n";
	for (BioSpecies* bioSpecies : species) {
		const std::string sbmlId = speciesToSbmlId[bioSpecies->getName()];
		out << "      <species id=\"" << encodeXmlEntities(sbmlId) << "\""
		    << " name=\"" << encodeXmlEntities(bioSpecies->getName()) << "\""
		    << " initialAmount=\"" << encodeXmlEntities(formatDouble(bioSpecies->getInitialAmount())) << "\""
		    << " constant=\"" << (bioSpecies->isConstant() ? "true" : "false") << "\""
		    << " boundaryCondition=\"" << (bioSpecies->isBoundaryCondition() ? "true" : "false") << "\"";
		if (!bioSpecies->getUnit().empty()) {
			out << " substanceUnits=\"" << encodeXmlEntities(bioSpecies->getUnit()) << "\"";
		}
		out << " />\n";
	}
	out << "    </listOfSpecies>\n";

	out << "    <listOfParameters>\n";
	for (BioParameter* bioParameter : parameters) {
		const std::string parameterId = parameterToSbmlId[bioParameter->getName()];
		out << "      <parameter id=\"" << encodeXmlEntities(parameterId) << "\""
		    << " name=\"" << encodeXmlEntities(bioParameter->getName()) << "\""
		    << " value=\"" << encodeXmlEntities(formatDouble(bioParameter->getValue())) << "\"";
		if (!bioParameter->getUnit().empty()) {
			out << " units=\"" << encodeXmlEntities(bioParameter->getUnit()) << "\"";
		}
		out << " />\n";
	}
	out << "    </listOfParameters>\n";

	out << "    <listOfReactions>\n";
	for (BioReaction* bioReaction : reactions) {
		const std::string reactionId = reactionToSbmlId[bioReaction->getName()];
		const std::string mappedForwardExpression = mapExpressionSymbols(
				bioReaction->getKineticLawExpression().empty()
				? reactionMassActionExpression(bioReaction, speciesToSbmlId, parameterToSbmlId)
				: bioReaction->getKineticLawExpression(),
				nativeToSbmlSymbols);
		std::string mappedReverseExpression = mapExpressionSymbols(bioReaction->getReverseKineticLawExpression(), nativeToSbmlSymbols);
		std::string mappedRateParameter = bioReaction->getRateConstantParameterName();
		if (!mappedRateParameter.empty()) {
			auto parameterIt = parameterToSbmlId.find(mappedRateParameter);
			if (parameterIt != parameterToSbmlId.end()) {
				mappedRateParameter = parameterIt->second;
			}
		}
		std::string mappedReverseRateParameter = bioReaction->getReverseRateConstantParameterName();
		if (!mappedReverseRateParameter.empty()) {
			auto parameterIt = parameterToSbmlId.find(mappedReverseRateParameter);
			if (parameterIt != parameterToSbmlId.end()) {
				mappedReverseRateParameter = parameterIt->second;
			}
		}

		out << "      <reaction id=\"" << encodeXmlEntities(reactionId) << "\""
		    << " name=\"" << encodeXmlEntities(bioReaction->getName()) << "\""
		    << " reversible=\"" << (bioReaction->isReversible() ? "true" : "false") << "\""
		    << " genesysKineticLawExpression=\"" << encodeXmlEntities(mappedForwardExpression) << "\"";
		if (!mappedRateParameter.empty()) {
			out << " genesysRateConstantParameter=\"" << encodeXmlEntities(mappedRateParameter) << "\"";
		}
		if (!bioReaction->isReversible()) {
			out << " genesysRateConstant=\"" << encodeXmlEntities(formatDouble(bioReaction->resolveRateConstant())) << "\"";
		}
		if (bioReaction->isReversible()) {
			if (!mappedReverseExpression.empty()) {
				out << " genesysReverseKineticLawExpression=\"" << encodeXmlEntities(mappedReverseExpression) << "\"";
			}
			if (!mappedReverseRateParameter.empty()) {
				out << " genesysReverseRateConstantParameter=\"" << encodeXmlEntities(mappedReverseRateParameter) << "\"";
			}
			if (mappedReverseExpression.empty() && mappedReverseRateParameter.empty()) {
				out << " genesysReverseRateConstant=\"" << encodeXmlEntities(formatDouble(bioReaction->resolveReverseRateConstant())) << "\"";
			}
		}
		out << ">\n";

		out << "        <listOfReactants>\n";
		for (const BioReaction::StoichiometricTerm& term : bioReaction->getReactants()) {
			auto speciesIt = speciesToSbmlId.find(term.speciesName);
			if (speciesIt == speciesToSbmlId.end()) {
				if (result != nullptr) {
					result->warnings.push_back("BioReaction \"" + bioReaction->getName() + "\" skipped reactant \"" + term.speciesName + "\" not present in exported BioNetwork.");
				}
				continue;
			}
			out << "          <speciesReference species=\"" << encodeXmlEntities(speciesIt->second)
			    << "\" stoichiometry=\"" << encodeXmlEntities(formatDouble(term.stoichiometry)) << "\" />\n";
		}
		out << "        </listOfReactants>\n";

		out << "        <listOfProducts>\n";
		for (const BioReaction::StoichiometricTerm& term : bioReaction->getProducts()) {
			auto speciesIt = speciesToSbmlId.find(term.speciesName);
			if (speciesIt == speciesToSbmlId.end()) {
				if (result != nullptr) {
					result->warnings.push_back("BioReaction \"" + bioReaction->getName() + "\" skipped product \"" + term.speciesName + "\" not present in exported BioNetwork.");
				}
				continue;
			}
			out << "          <speciesReference species=\"" << encodeXmlEntities(speciesIt->second)
			    << "\" stoichiometry=\"" << encodeXmlEntities(formatDouble(term.stoichiometry)) << "\" />\n";
		}
		out << "        </listOfProducts>\n";

		if (!bioReaction->getModifiers().empty()) {
			out << "        <listOfModifiers>\n";
			for (const std::string& modifier : bioReaction->getModifiers()) {
				auto speciesIt = speciesToSbmlId.find(modifier);
				if (speciesIt == speciesToSbmlId.end()) {
					if (result != nullptr) {
						result->warnings.push_back("BioReaction \"" + bioReaction->getName() + "\" skipped modifier \"" + modifier + "\" not present in exported BioNetwork.");
					}
					continue;
				}
				out << "          <modifierSpeciesReference species=\"" << encodeXmlEntities(speciesIt->second) << "\" />\n";
			}
			out << "        </listOfModifiers>\n";
		}

		out << "        <kineticLaw formula=\"" << encodeXmlEntities(mappedForwardExpression)
		    << "\" genesysRateConstant=\"" << encodeXmlEntities(formatDouble(bioReaction->resolveRateConstant())) << "\">\n";
		out << "          <math>" << encodeXmlEntities(mappedForwardExpression) << "</math>\n";
		out << "        </kineticLaw>\n";
		out << "      </reaction>\n";
	}
	out << "    </listOfReactions>\n";

	out << "  </model>\n";
	out << "</sbml>\n";

	*sbmlText = out.str();
	if (result != nullptr) {
		result->networkName = network->getName();
		result->speciesExported = static_cast<unsigned int>(species.size());
		result->parametersExported = static_cast<unsigned int>(parameters.size());
		result->reactionsExported = static_cast<unsigned int>(reactions.size());
	}
	return true;
}
