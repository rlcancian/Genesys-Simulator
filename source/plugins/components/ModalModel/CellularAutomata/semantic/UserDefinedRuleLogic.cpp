#include "UserDefinedRuleLogic.h"
#include <sstream>

bool UserDefinedRuleLogic::_loadInstance(PersistenceRecord* fields) {
    if (fields == nullptr) {
        return false;
    }

    // Carrega sourceCode
    sourceCode = fields->loadField("userRule.sourceCode", std::string(""));

    // Carrega parâmetros
    parameters.clear();
    std::string paramsStr = fields->loadField("userRule.parameters", std::string(""));
    if (!paramsStr.empty()) {
        std::istringstream iss(paramsStr);
        std::string pair;
        while (std::getline(iss, pair, ';')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string name = pair.substr(0, pos);
                double value = std::stod(pair.substr(pos + 1));
                parameters[name] = value;
            }
        }
    }

    compiled = false;
    compilationError.clear();
    return true;
}

void UserDefinedRuleLogic::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    if (fields == nullptr) {
        return;
    }

    // Salva sourceCode
    fields->saveField("userRule.sourceCode", sourceCode, std::string(""), saveDefaultValues);

    // Salva parâmetros como string "name1=value1;name2=value2;..."
    std::ostringstream oss;
    for (const auto& pair : parameters) {
        if (oss.tellp() > 0) oss << ";";
        oss << pair.first << "=" << pair.second;
    }
    fields->saveField("userRule.parameters", oss.str(), std::string(""), saveDefaultValues);
}
