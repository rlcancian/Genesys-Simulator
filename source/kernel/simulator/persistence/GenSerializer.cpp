#include "GenSerializer.h"

#include <cassert>
#include <cctype>
#include <string_view>
#include <vector>
#include <algorithm>

#include "../Simulator.h"


namespace {

bool requiresEscapedTextLiteral(std::string_view value) {
    return value.find('"') != std::string_view::npos
        || value.find('\n') != std::string_view::npos
        || value.find('\r') != std::string_view::npos
        || value.find('\t') != std::string_view::npos;
}

std::string encodeTextLiteral(std::string_view value) {
    if (!requiresEscapedTextLiteral(value)) {
        return "\"" + std::string(value) + "\"";
    }

    std::string encoded;
    encoded.reserve(value.size() + 3);
    encoded += "e\"";
    for (const char ch : value) {
        switch (ch) {
        case '\\':
            encoded += "\\\\";
            break;
        case '"':
            encoded += "\\\"";
            break;
        case '\n':
            encoded += "\\n";
            break;
        case '\r':
            encoded += "\\r";
            break;
        case '\t':
            encoded += "\\t";
            break;
        default:
            encoded += ch;
            break;
        }
    }
    encoded += '"';
    return encoded;
}

void skipWhitespace(std::string_view line, std::size_t& position) {
    while (position < line.size()
           && std::isspace(static_cast<unsigned char>(line[position])) != 0) {
        ++position;
    }
}

bool parseBareToken(std::string_view line, std::size_t& position, std::string& value) {
    skipWhitespace(line, position);
    const std::size_t start = position;
    while (position < line.size()
           && std::isspace(static_cast<unsigned char>(line[position])) == 0) {
        ++position;
    }
    if (position == start) {
        return false;
    }
    value.assign(line.substr(start, position - start));
    return true;
}

bool parseTextOrBareValue(std::string_view line,
                          std::size_t& position,
                          std::string& value,
                          bool& isText) {
    skipWhitespace(line, position);
    if (position >= line.size()) {
        value.clear();
        isText = false;
        return true;
    }

    const bool escapedLiteral =
        line[position] == 'e'
        && position + 1 < line.size()
        && line[position + 1] == '"';
    const bool quotedLiteral = line[position] == '"';

    if (!escapedLiteral && !quotedLiteral) {
        isText = false;
        return parseBareToken(line, position, value);
    }

    isText = true;
    position += escapedLiteral ? 2 : 1;
    value.clear();

    while (position < line.size()) {
        const char ch = line[position++];
        if (ch == '"') {
            return true;
        }

        if (!escapedLiteral || ch != '\\') {
            value += ch;
            continue;
        }

        if (position >= line.size()) {
            return false;
        }

        const char escaped = line[position++];
        switch (escaped) {
        case '"':
            value += '"';
            break;
        case '\\':
            value += '\\';
            break;
        case 'n':
            value += '\n';
            break;
        case 'r':
            value += '\r';
            break;
        case 't':
            value += '\t';
            break;
        default:
            return false;
        }
    }

    return false;
}

void restoreLegacySpaceEscapes(std::string& value) {
    constexpr std::string_view marker = "\\_";
    std::size_t position = 0;
    while ((position = value.find(marker, position)) != std::string::npos) {
        value.replace(position, marker.size(), " ");
        ++position;
    }
}

} // namespace

GenSerializer::GenSerializer(Model *model) :
_model(model) {
	assert(model != nullptr);
}

PersistenceRecord* GenSerializer::newPersistenceRecord() {
	return new PersistenceRecord(*_model->getPersistence());
}

bool GenSerializer::dump(std::ostream& output) {
	auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
	bool found, err;

	output << "# Genesys Simulation Model\n";
	output << "# Simulator, Model and Simulation infos\n";

	fields->clear();
	found = get("SimulatorInfo", fields.get()) ? true : get("Simulator", fields.get());
	if (found) output << linearize(fields.get());

	fields->clear();
	found = get("ModelInfo", fields.get());
	if (found) output << linearize(fields.get());

	fields->clear();
	found = get("ModelSimulation", fields.get());
	if (found) output << linearize(fields.get());

	output << "\n# Model Data Definitions\n";
	err = for_each([&](auto& key) {
		fields->clear();
		get(key, fields.get());
		auto type = fields->loadField("typename");
		if (type == "Simulator" || type == "SimulatorInfo" || type == "ModelInfo" || type == "ModelSimulation") return 0;
				Plugin * plugin = _model->getParentSimulator()->getPluginManager()->find(type);
			if (plugin == nullptr) return 1;
				if (plugin->getPluginInfo()->isComponent()) return 0;
						_model->getTracer()->trace(linearize(fields.get()));
						output << linearize(fields.get());
					return 0;
				});

	output << "\n# Model Components\n";
	err = for_each([&](auto& key) {
		fields->clear();
		get(key, fields.get());
		auto type = fields->loadField("typename");
		if (type == "Simulator" || type == "SimulatorInfo" || type == "ModelInfo" || type == "ModelSimulation") return 0;
				Plugin * plugin = _model->getParentSimulator()->getPluginManager()->find(type);
			if (plugin == nullptr) return 1;
				if (!plugin->getPluginInfo()->isComponent()) return 0;
						_model->getTracer()->trace(linearize(fields.get()));
						output << linearize(fields.get());
					return 0;
				});

	return !err;
}

std::string GenSerializer::linearize(PersistenceRecord *fields) {
	std::string id, type, name, attrs;
	for (auto& it : *fields) {
		auto field = it.second;
		if (field.first == "id") {
			id = field.second;
		} else if (field.first == "typename") {
			type = field.second;
		} else if (field.first == "name") {
			name = encodeTextLiteral(field.second);
		} else {
			const auto& key = field.first;
			const std::string serializedValue =
				field.kind == PersistenceRecord::Entry::Kind::text
					? encodeTextLiteral(field.second)
					: field.second;
			attrs += key + "=" + serializedValue + " ";
		}
	}

	while (id.length() < 3) id += " ";
	while (type.length() < 10) type += " ";

	return id + " " + type + " " + name + " " + attrs + "\n";
};

bool GenSerializer::load(std::istream& input) {
	bool res = true;
	std::string line;
	while (std::getline(input, line) && res) {
		line = Util::Trim(line);
		if (line.empty() || line.front() == '#') {
			continue;
		}

		_model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, line);

		std::size_t position = 0;
		std::string idToken;
		std::string type;
		std::string name;
		bool nameIsText = false;

		if (!parseBareToken(line, position, idToken)
			|| !parseBareToken(line, position, type)
			|| !parseTextOrBareValue(line, position, name, nameIsText)) {
			return false;
		}

		auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
		fields->insert({"id", idToken, PersistenceRecord::Entry::Kind::numeric});
		fields->insert({"typename", type, PersistenceRecord::Entry::Kind::text});
		fields->insert({"name", name, PersistenceRecord::Entry::Kind::text});

		while (true) {
			skipWhitespace(line, position);
			if (position >= line.size()) {
				break;
			}

			const std::size_t keyStart = position;
			while (position < line.size()
				   && line[position] != '='
				   && std::isspace(static_cast<unsigned char>(line[position])) == 0) {
				++position;
			}
			if (position == keyStart) {
				return false;
			}

			const std::string key(line.substr(keyStart, position - keyStart));
			skipWhitespace(line, position);
			if (position >= line.size() || line[position] != '=') {
				return false;
			}
			++position;

			std::string value;
			bool isText = false;
			if (!parseTextOrBareValue(line, position, value, isText)) {
				return false;
			}
			if (!isText) {
				restoreLegacySpaceEscapes(value);
			}

			fields->insert({
				key,
				value,
				isText ? PersistenceRecord::Entry::Kind::text
				       : PersistenceRecord::Entry::Kind::numeric
			});
		}

		if (type.empty()) {
			return false;
		}
		const Util::identification id = fields->loadField("id", 0);
		const std::string recordName =
			id == 0 ? type : fields->loadField("name", "_" + std::to_string(id));
		res = put(recordName, type, id, fields.get());
	}
	return res;
}

bool GenSerializer::get(const std::string& name, PersistenceRecord *entry) {
	assert(entry != nullptr);
	auto it = _components.find(name);
	if (it == _components.end()) return false;
	entry->insert(it->second->begin(), it->second->end());
	return true;
}

bool GenSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
	assert(fields != nullptr);
	auto saved = std::unique_ptr<PersistenceRecord>(this->newPersistenceRecord());
	saved->insert(fields->begin(), fields->end());
	if (id != 0) saved->saveField("name", name);
	saved->saveField("typename", type);
	saved->saveField("id", id);
	if (_components.find(name) != _components.end()) {
		// Two live objects ended up saved under the same name; this map is keyed by name
		// alone across both data definitions and components, so the earlier entry is about
		// to be silently discarded. Model-wide uniqueness should already be enforced at
		// ModelDataDefinition::setName(), so this is a defense-in-depth diagnostic for any
		// residual/legacy case that bypassed it.
		_model->getTracer()->traceError(
			"Saved model has two objects named \"" + name + "\" (typename \"" + type
			+ "\"); the earlier entry will be overwritten and lost.");
	}
	_components[name] = std::move(saved);
	return true;
}

int GenSerializer::for_each(std::function<int(const std::string&) > delegate) {
	// enfore id-order
	std::vector<std::string> sorted;
	sorted.reserve(_components.size());
	for (auto& entry : _components) sorted.push_back(entry.first);
	std::sort(sorted.begin(), sorted.end(), [&](auto& a, auto& b) {
		return this->_components.at(a)->loadField("id", -1) < this->_components.at(b)->loadField("id", -1);
	});

	// then do the user-level iteration
	for (auto& label : sorted) {
		int stop = delegate(label);
		if (stop) return stop;
	}
	return 0;
}
