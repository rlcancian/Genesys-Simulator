#include "XmlSerializer.h"

#include <cassert>
#include <regex>
#include <sstream>
#include <map>
#include <vector>
#include <stack>
#include <cctype>
#include <algorithm>
#include <utility>

#define INDENT "\t"

XmlSerializer::XmlSerializer(Model *model) :
_model(model) {
	assert(model != nullptr);
}

PersistenceRecord* XmlSerializer::newPersistenceRecord() {
	return new PersistenceRecord(*_model->getPersistence());
}

static std::string prettyPrint(const std::string& key, PersistenceRecord *fields, unsigned indentationLevel, bool meta) {
	std::string indentation = "";
	for (unsigned i = 0; i < indentationLevel; ++i) indentation += INDENT;

	// write header
	std::string xml = "";
	if (meta) {
		xml += indentation + "<" + key + ">";
	} else {
		Util::identification id = fields->loadField("id", -1);
		std::string name = fields->loadField("name", "");
		xml += indentation + "<" + key + " name=\"" + name + "\" id=\"" + std::to_string(id) + "\">";
	}

	// write fields
	for (auto it = fields->begin(); it != fields->end(); ++it) {
		std::string field = it->first;
		if (field == "typename" || field == "id" || (field == "name" && !meta)) continue;
		xml += "\n" + indentation + INDENT;
		// check for indexed field
		int index = -1;
		std::smatch match;
		if (std::regex_search(field, match, std::regex("\\[(\\d+)\\]$"))) {
			index = std::stoi(match[1]);
			field = match.prefix();
			xml += "<" + field + " index=\"" + std::to_string(index) + "\">";
		} else {
			xml += "<" + field + ">";
		}
		// escape especial sequences in text
		std::string value = it->second.second;
		value = std::regex_replace(value, std::regex("&"), "&amp;");
		value = std::regex_replace(value, std::regex("\""), "&quot;");
		value = std::regex_replace(value, std::regex("'"), "&apos;");
		value = std::regex_replace(value, std::regex("<"), "&lt;");
		value = std::regex_replace(value, std::regex(">"), "&gt;");
		xml += value;
		xml += "</" + field + ">";
	}

	// finalize object
	xml += "\n" + indentation + "</" + key + ">\n";
	return xml;
}

bool XmlSerializer::dump(std::ostream& output) {
	auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());

	output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	output << "<GenesysModel>\n";

	for (auto& entry : _metaobjects) {
		output << prettyPrint(entry.first, entry.second.get(), 1, true) << "\n";
	}

	output << "\n" << INDENT << "<Components>\n";
	for (auto& entry : _components) {
		auto key = entry.second->loadField("typename");
		output << prettyPrint(key, entry.second.get(), 2, false) << "\n";
	}
	output << INDENT << "</Components>\n";

	output << "</GenesysModel>\n";

	return true;
}

bool XmlSerializer::load(std::istream& input) {
	// read the entire file
	std::string xml;
	{
		std::stringstream buffer;
		buffer << input.rdbuf();
		xml = buffer.str();
	}

	// every element will be parsed as one of these

	struct Xml {
		std::string tag;
		std::map<std::string, std::string> attributes{};
		bool leaf{true};
		// only if leaf
		std::string payload{""};
		// only if non-leaf
		std::vector<std::unique_ptr<Xml>> children
		{
		};
	};
	std::vector<Xml> elements;

	// helper macros
#define HALT(msg) do { \
        _model->getTracer()->traceError((msg)); \
        return false; \
    } while (0)
#define NOT_EOF(pos) do { \
        if ((pos) >= xml.length()) HALT("invalid XML: unexpected end of file"); \
    } while (0)
#define EXPECT(pos, expected) do { \
        NOT_EOF(pos); \
        if (xml[(pos)] != (expected)) { \
            HALT(std::string("invalid XML: expected '") + (expected) + "' but found '" + xml[(pos)] + "'"); \
        } \
    } while (0)

	// unescapes
	const auto unescape = [](const std::string & original) {
		std::string str = original;
		str = std::regex_replace(str, std::regex("&amp;"), "&");
		str = std::regex_replace(str, std::regex("&quot;"), "\"");
		str = std::regex_replace(str, std::regex("&apos;"), "'");
		str = std::regex_replace(str, std::regex("&lt;"), "<");
		str = std::regex_replace(str, std::regex("&gt;"), ">");
		return str;
	};

	// parse with stack for balance check
	std::stack<Xml> stack;
	for (std::size_t cursor = 0u; cursor < xml.length();) {
		// ignore whitespace
		while (cursor < xml.length() && std::isspace(xml[cursor])) ++cursor;
		if (cursor >= xml.length()) break;

		// check for tag
		const auto tag_open = cursor;
		EXPECT(tag_open, '<');

		NOT_EOF(tag_open + 1);
		// ignore <!-- comments -->
		if (xml[tag_open + 1] == '!') {
			EXPECT(tag_open + 2, '-');
			EXPECT(tag_open + 3, '-');
			cursor = xml.find("-->") + 3;
			NOT_EOF(cursor);
			continue;
			// skip <?xml meta-element ?>
		} else if (xml[tag_open + 1] == '?') {
			cursor = xml.find("?>") + 2;
			NOT_EOF(cursor);
			continue;
		}

		// look for end of tag
		const auto tag_close = xml.find('>', tag_open);
		NOT_EOF(tag_close);

		// extract raw tag contents: prefix + tagname + whitespace-separated attributes
		auto tag = xml.substr(tag_open + 1, tag_close - 1 - tag_open);

		// closing tag? check top of stack
		if (tag[0] == '/') {
			if (stack.empty() || stack.top().tag != tag.substr(1)) {
				HALT("invalid XML: found unexpected closing tag <" + tag + ">");
			}
			// if ok, finalize the current element
			auto finalized = std::move(stack.top());
			stack.pop();
			if (finalized.leaf) _model->getTracer()->trace(finalized.payload);
			Util::DecIndent();
			// then, either add it to the outer element, or to the toplevel
			if (stack.empty()) {
				elements.push_back(std::move(finalized));
			} else {
				// if this was added to an outer element, that outer thing cannot be a leaf
				if (Util::Trim(stack.top().payload) != "") {
					HALT("invalid XML syntax in subtree of <" + stack.top().tag + ">: \"" + stack.top().payload + "\"");
				}
				stack.top().children.push_back(std::make_unique<Xml>(std::move(finalized)));
				stack.top().leaf = false;
			}
			cursor = tag_close + 1;
			continue;
		}

		// extract tagname
		std::smatch match;
		if (!std::regex_search(tag, match, std::regex("^([A-Za-z_]\\w*)\\s*"))) {
			HALT("invalid XML tag: \"" + tag + "\"");
		}
		std::string tagname = match[1];

		// extract attributes from suffix
		std::map<std::string, std::string> attributes{};
		std::string attrs = match.suffix();
		while (attrs.length() > 0) {
			if (!std::regex_search(attrs, match, std::regex("^([A-Za-z_]\\w*)=\"(.*?)\"(?:\\s+|$)"))) {
				HALT("invalid XML attribute: \"" + attrs + "\"");
			}
			std::string key = match[1];
			std::string value = match[2];
			attributes[key] = unescape(value);
			attrs = match.suffix();
		}

		// consume payload up to next tag
		std::string payload = "";
		for (cursor = tag_close + 1; cursor < xml.length() && xml[cursor] != '<'; ++cursor) {
			payload += xml[cursor];
		}
		if (cursor >= xml.length()) break;

		// push the just-opened element
		Xml element = {.tag = tagname, .attributes = attributes, .leaf = true, .payload = unescape(payload)};
		stack.push(std::move(element));
		_model->getTracer()->trace("<" + tag + ">");
		Util::IncIndent();
	}
	if (!stack.empty()) HALT("invalid XML: unclosed tags detected");

	// now we actually interpret the parsed (and already unescaped) XML
	const auto xml2fields = [](PersistenceRecord *fields, const Xml & xml) {
		bool ok = true;

		// attributes contain name and id
		for (auto& attribute : xml.attributes) {
			auto kind = PersistenceRecord::Entry::Kind::text;
			if (std::regex_match(attribute.second, std::regex("^-?(?:[1-9]\\d+|\\d)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?$"))) {
				kind = PersistenceRecord::Entry::Kind::numeric;
			}
			fields->insert({attribute.first, attribute.second, kind});
		}

		// children are assumed to be leafs, tags as field names (maybe including an index attribute)
		for (auto& field : xml.children) {
			if (!field->leaf) ok = false;
			auto name = field->tag;
			auto it = field->attributes.find("index");
			if (it != field->attributes.end()) {
				int index = std::stoi(it->second);
				name = field->tag + Util::StrIndex(index);
			}
			auto kind = PersistenceRecord::Entry::Kind::text;
			if (std::regex_match(field->payload, std::regex("^-?(?:[1-9]\\d+|\\d)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?$"))) {
				kind = PersistenceRecord::Entry::Kind::numeric;
			}
			fields->insert({name, field->payload, kind});
		}

		// record tag = typename
		fields->saveField("typename", xml.tag);

		return ok;
	};
	if (elements.size() != 1 || elements[0].tag != "GenesysModel") {
		HALT("invalid XML model: expected a single <GenesysModel> in the toplevel");
	}
	auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
	bool ok = true;
	for (auto& element : elements[0].children) {
		if (element->tag != "Components") {
			fields->clear();
			ok &= xml2fields(fields.get(), *element);
			auto name = fields->loadField("name", element->tag);
			put(name, element->tag, 0, fields.get());
			continue;
		}
		for (auto& component : element->children) {
			fields->clear();
			ok &= xml2fields(fields.get(), *component);
			auto name = fields->loadField("name");
			Util::identification id = fields->loadField("id", -1);
			put(name, component->tag, id, fields.get());
		}
	}

	return ok;
}

bool XmlSerializer::get(const std::string& name, PersistenceRecord *entry) {
	assert(entry != nullptr);
	auto key = name == "Simulator" ? "SimulatorInfo" : name;

	auto it = _metaobjects.find(key);
	if (it != _metaobjects.end()) {
		entry->insert(it->second->begin(), it->second->end());
		return true;
	}

	it = _components.find(key);
	if (it == _components.end()) return false;
	entry->insert(it->second->begin(), it->second->end());
	return true;
}

bool XmlSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
	// copy fields
	assert(fields != nullptr);
	auto saved = std::unique_ptr<PersistenceRecord>(this->newPersistenceRecord());
	saved->insert(fields->begin(), fields->end());

	// enforce consistency
	if (type == "") return false;
	auto unaliased = type == "Simulator" ? "SimulatorInfo" : type;
	saved->saveField("typename", unaliased);
	saved->saveField("id", id);

	// choose partition
	if (id == 0) {
		_metaobjects[unaliased] = std::move(saved);
	} else {
		if (id != 0) saved->saveField("name", name);
		_components[name] = std::move(saved);
	}

	return true;
}

int XmlSerializer::for_each(std::function<int(const std::string&) > delegate) {
	// enfore id-order
	std::vector < std::pair<bool, std::string>> sorted;
	sorted.reserve(_metaobjects.size() + _components.size());
	for (auto& entry : _metaobjects) sorted.push_back({true, entry.first});
	for (auto& entry : _components) sorted.push_back({false, entry.first});
	std::sort(sorted.begin(), sorted.end(), [&](auto& a, auto& b) {
		if (a.first && b.first) { // both metatypes
			return this->_metaobjects.at(a.second)->loadField("id", 0) < this->_metaobjects.at(b.second)->loadField("id", 0);
		} else if (!a.first && !b.first) {
			return this->_components.at(a.second)->loadField("id", -1) < this->_components.at(b.second)->loadField("id", -1);
		} else /* different origins? metatypes < components */ {
			return a.first && !b.first;
		}
	});

	// then do the user-level iteration
	for (auto& e : sorted) {
		int stop = delegate(e.second);
		if (stop) return stop;
	}
	return 0;
}
