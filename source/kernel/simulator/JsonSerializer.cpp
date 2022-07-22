#include "JsonSerializer.h"

#include <cassert>
#include <regex>
#include <map>
#include <vector>
#include <memory>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <utility>

#define INDENT "\t"


JsonSerializer::JsonSerializer(Model *model):
    _model(model)
{
    assert(model != nullptr);
}


PersistenceRecord* JsonSerializer::newPersistenceRecord() {
    return new PersistenceRecord(*_model->getPersistence());
}


static std::string prettyPrint(const std::string& key, PersistenceRecord *fields, unsigned indentationLevel) {
    std::string indentation = "";
    for (unsigned i = 0; i < indentationLevel; ++i) indentation += INDENT;
    
    // open obj
    std::string json = indentation + "\"" + key + "\": {\n";

    const auto escapeText = [](const std::string& original){
        return std::regex_replace(original, std::regex("\""), "\\\"");
    };

    // write fields and aggregate fields
    std::unordered_map<std::string, std::map<unsigned, std::string>> seqs;
    for (auto it = fields->begin(); it != fields->end();) {
        auto field = it->second.first;
        auto value = it->second.second;
        auto kind = it->second.kind;
        std::smatch match;
        if (std::regex_search(field, match, std::regex("\\[(\\d+)\\]$"))) {
            unsigned index = std::stoi(match[1]);
            std::string name  = match.prefix();
            seqs[name][index] = kind == PersistenceRecord::Entry::Kind::text ? "\"" + escapeText(value) + "\"" : value;
            ++it;
        } else {
            json += indentation + INDENT + "\"" + field + "\": ";
            json += kind == PersistenceRecord::Entry::Kind::text ? "\"" + escapeText(value) + "\"" : value;
            ++it;
            json += it == fields->end() && seqs.empty() ? "\n" : ",\n";
        }
    }

    // write sequences (if any)
    for (auto i = seqs.begin(); i != seqs.end();) {
        json += indentation + INDENT + "\"" + i->first + "\": [";
        // this iteration is ordered on indexes, since std::map is ordered
        auto& seq = i->second;
        for (auto j = seq.begin(); j != seq.end();) {
            auto value = j->second;
            json += value;
            ++j;
            json += j == seq.end() ? "" : ", ";
        }
        json += "]";
        ++i;
        json += i == seqs.end() ? "\n" : ",\n";
    }

    // finalize obj
    json += indentation + "}";
    return json;
}


bool JsonSerializer::dump(std::ostream& output) {
    auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());

    output << "{\n";

    for (auto& entry : _metaobjects) {
        output << prettyPrint(entry.first, entry.second.get(), 1) << ",\n";
    }

    output << INDENT << "\"Components\": {\n";
    for (auto it = _components.begin(); it != _components.end();) {
        // temporarily remove redundante "name" field before pretty print
        auto& fields = it->second;
        fields->erase("name");
        output << prettyPrint(it->first, fields.get(), 2);
        fields->saveField("name", it->first);
        ++it;
        output << (it == _components.end() ? "\n" : ",\n");
    }
    output << INDENT << "}\n";

    output << "}\n";

    return true;
}


// this is the (LL1) grammar which we'll be parsing in predictive-rec-desc style
/*
    <json>      ::= "false" | "null" | NUMBER | STRING | "true" | <array> | <object>
    <object>    ::= '{' <members> '}'
    <members>   ::= & | STRING ':' <json> <members_>
    <members_>  ::= & | ',' <members>
    <array>     ::= '[' <elements> ']'
    <elements>  ::= & | <json> <elements_>
    <elements_> ::= & | ',' <elements>
*/
// see below for token regex definitions
static const std::regex::flag_type FLAGS = std::regex_constants::optimize | std::regex_constants::ECMAScript;
static const std::regex number_pattern("^(-?(?:[1-9]\\d+|\\d)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?)", FLAGS);
static const std::regex string_pattern("^(\"\"|\".*?[^\\\\]\")", FLAGS);

struct Token {
    enum Kind { eof, null, f, t, number, string, lbracket, rbracket, lbrace, rbrace, comma, colon } kind;
    std::string str{}; // only if number of string
};

static Token lex(const std::string& input, std::size_t& pos) {
    // skip whitespace
    while (pos < input.length() && std::isspace(input[pos])) ++pos;
    if (pos >= input.length()) return { Token::Kind::eof, "\0" };

    // helpers
    #define EXPECT(i, c) do { \
        if ((i) >= input.length() || input[(i)] != (c)) { \
            throw std::runtime_error( \
                "invalid JSON: expected '" + std::to_string(c) + \
                "' but found " + ((i) >= input.length() ? "EOF" : "'" + std::to_string(input[(i)]) + "'") \
            ); \
        } \
    } while (0) 

    // switch on prefix
    switch (input[pos]) {
        case '[':
            pos += 1;
            return { Token::Kind::lbracket, "[" };
        case ']':
            pos += 1;
            return { Token::Kind::rbracket, "]" };
        case '{':
            pos += 1;
            return { Token::Kind::lbrace, "{" };
        case '}':
            pos += 1;
            return { Token::Kind::rbrace, "}" };
        case ',':
            pos += 1;
            return { Token::Kind::comma, "," };
        case ':':
            pos += 1;
            return { Token::Kind::colon, ":" };
        case 'n':
            EXPECT(pos + 0, 'n');
            EXPECT(pos + 1, 'u');
            EXPECT(pos + 2, 'l');
            EXPECT(pos + 3, 'l');
            pos += 4;
            return { Token::Kind::null, "null" };
        case 'f':
            EXPECT(pos + 0, 'f');
            EXPECT(pos + 1, 'a');
            EXPECT(pos + 2, 'l');
            EXPECT(pos + 3, 's');
            EXPECT(pos + 4, 'e');
            pos += 5;
            return { Token::Kind::f, "false" };
        case 't':
            EXPECT(pos + 0, 't');
            EXPECT(pos + 1, 'r');
            EXPECT(pos + 2, 'u');
            EXPECT(pos + 3, 'e');
            pos += 4;
            return { Token::Kind::t, "true" };
        case '"': {
            std::smatch match;
            if (!std::regex_search(input.begin() + pos, input.end(), match, string_pattern)) {
                throw std::runtime_error("invalid JSON string: \"" + input.substr(pos) + "\"");
            }
            std::string unescaped = match[1];
            pos += unescaped.length(); // move cursor
            unescaped = unescaped.substr(1, unescaped.length() - 2); // remove outer quotes
            unescaped = std::regex_replace(unescaped, std::regex("\\\\\""), "\""); // unescape inner quotes
            unescaped = std::regex_replace(unescaped, std::regex("\\\\\\\\"), "\\"); // unescape escape char
            return { Token::Kind::string, unescaped };
        }
        default: {
            std::smatch match;
            if (!std::regex_search(input.begin() + pos, input.end(), match, number_pattern)) {
                throw std::runtime_error("invalid JSON number: \"" + input.substr(pos) + "\"");
            }
            std::string literal = match[1];
            pos += literal.length(); // move cursor
            return { Token::Kind::number, literal };
        }
    }

    #undef EXPECT
}

struct Json {
    enum Kind { null, boolean, number, string, array, object } kind;
    // used for booleans (0 = false) and numbers
    double value;
    // only for strings
    std::string text;
    // only for arrays
    std::vector<std::unique_ptr<Json>> sequence{};
    // only for objects
    std::unordered_map<std::string, std::unique_ptr<Json>> fields{};
};

static void halt(const std::string msg) {
    throw std::runtime_error("invalid JSON: " + msg);
}

static void check(Token::Kind expected, const Token& tok) {
    if (tok.kind != expected) {
        halt("expected token type #" + std::to_string(expected) + " but found #" + std::to_string(tok.kind) + "(" + tok.str + ")");
    }
}

// forward decls
static Json json(const std::string& input, std::size_t& pos, Token& lookahead);
static Json object(const std::string& input, std::size_t& pos, Token& lookahead);
static void members(const std::string& input, std::size_t& pos, Token& lookahead, std::unordered_map<std::string, std::unique_ptr<Json>>& fields);
static void members_(const std::string& input, std::size_t& pos, Token& lookahead, std::unordered_map<std::string, std::unique_ptr<Json>>& fields);
static Json array(const std::string& input, std::size_t& pos, Token& lookahead);
static void elements(const std::string& input, std::size_t& pos, Token& lookahead, std::vector<std::unique_ptr<Json>>& seq);
static void elements_(const std::string& input, std::size_t& pos, Token& lookahead, std::vector<std::unique_ptr<Json>>& seq);

static Json json(const std::string& input, std::size_t& pos, Token& lookahead) {
    switch (lookahead.kind) {
        case Token::Kind::f: // <json> ::= false
            lookahead = lex(input, pos);
            return { Json::Kind::boolean, .value = 0 };
        case Token::Kind::lbrace: // <json> ::= <object>
            return object(input, pos, lookahead);
        case Token::Kind::lbracket: // <json> ::= <array>
            return array(input, pos, lookahead);
        case Token::Kind::null: // <json> ::= null
            lookahead = lex(input, pos);
            return { Json::Kind::null };
        case Token::Kind::number: { // <json> ::= number
            auto value = std::stod(lookahead.str);
            lookahead = lex(input, pos);
            return { Json::Kind::number, .value = value };
        }
        case Token::Kind::string: { // <json> ::= string
            auto text = lookahead.str;
            lookahead = lex(input, pos);
            return { Json::Kind::string, .text = text };
        }
        case Token::Kind::t: // <json> ::= true
            lookahead = lex(input, pos);
            return { Json::Kind::boolean, .value = 1 };
        default:
            halt("unexpected token at `json` rule");
            return {};
    }
}

static Json object(const std::string& input, std::size_t& pos, Token& lookahead) {
    switch (lookahead.kind) {
        case Token::Kind::lbrace: { // <object> ::= lbrace <members> rbrace
            Json obj = { Json::Kind::object };
            lookahead = lex(input, pos);
            members(input, pos, lookahead, obj.fields);
            check(Token::Kind::rbrace, lookahead);
            lookahead = lex(input, pos);
            return obj;
        }
        default:
            halt("unexpected token at `object` rule");
            return {};
    }
}

static void members(const std::string& input, std::size_t& pos, Token& lookahead, std::unordered_map<std::string, std::unique_ptr<Json>>& fields) {
    switch (lookahead.kind) {
        case Token::Kind::string: { // <members> ::= string colon <json> <members_>
            auto key = lookahead.str;
            lookahead = lex(input, pos);
            check(Token::Kind::colon, lookahead);
            lookahead = lex(input, pos);
            auto value = json(input, pos, lookahead);
            fields[key] = std::make_unique<Json>(std::move(value));
            return members_(input, pos, lookahead, fields);
        }
        case Token::Kind::rbrace: // <members> ::= ε
            return;
        default:
            halt("unexpected token at `members` rule");
            return;
    }
}

static void members_(const std::string& input, std::size_t& pos, Token& lookahead, std::unordered_map<std::string, std::unique_ptr<Json>>& fields) {
    switch (lookahead.kind) {
        case Token::Kind::comma: { // <members_> ::= comma <members>
            lookahead = lex(input, pos);
            return members(input, pos, lookahead, fields);
        }
        case Token::Kind::rbrace: // <members_> ::= ε
            return;
        default:
            halt("unexpected token at `members_` rule");
            return;
    }
}

static Json array(const std::string& input, std::size_t& pos, Token& lookahead) {
    switch (lookahead.kind) {
        case Token::Kind::lbracket: { // <array> ::= lbracket <elements> rbracket
            Json seq = { Json::Kind::array };
            lookahead = lex(input, pos);
            elements(input, pos, lookahead, seq.sequence);
            check(Token::Kind::rbracket, lookahead);
            lookahead = lex(input, pos);
            return seq;
        }
        default:
            halt("unexpected token at `array` rule");
            return {};
    }
}

static void elements(const std::string& input, std::size_t& pos, Token& lookahead, std::vector<std::unique_ptr<Json>>& seq) {
    switch (lookahead.kind) {
        case Token::Kind::rbracket: // <elements> ::= ε
            return;
        // <elements> ::= <json> <elements_>
        case Token::Kind::f:
        case Token::Kind::lbrace:
        case Token::Kind::lbracket:
        case Token::Kind::null:
        case Token::Kind::number:
        case Token::Kind::string:
        case Token::Kind::t:
        {
            auto element = json(input, pos, lookahead);
            seq.push_back(std::make_unique<Json>(std::move(element)));
            elements_(input, pos, lookahead, seq);
            return;
        }
        default:
            halt("unexpected token at `elements` rule");
            return;
    }
}

static void elements_(const std::string& input, std::size_t& pos, Token& lookahead, std::vector<std::unique_ptr<Json>>& seq) {
    switch (lookahead.kind) {
        case Token::Kind::comma: // <elements_> ::= comma <elements>
            lookahead = lex(input, pos);
            return elements(input, pos, lookahead, seq);
        case Token::Kind::rbracket: // <elements_> ::= ε
            return;
        default:
            halt("unexpected token at `elements_` rule");
            return;
    }
}

bool JsonSerializer::load(std::istream& input) {
    // read the entire file
    std::string src;
    {
        std::stringstream buffer;
        buffer << input.rdbuf();
        src = buffer.str();
    }

    const auto json2fields = [](PersistenceRecord *fields, std::string name, const Json& js){
        // ensure we're dealing with an object
        if (js.kind != Json::Kind::object) halt("expected an object to describe component \"" + name + "\"");
        for (auto& field : js.fields) {
            auto key = field.first;
            auto& val = field.second;
            switch (val->kind) {
                case Json::Kind::null:
                case Json::Kind::object:
                    halt("invalid type for field \"" + key + "\"");
                    break;
                case Json::Kind::boolean:
                    fields->saveField(key, (val->value != 0));
                    break;
                case Json::Kind::number:
                    fields->saveField(key, val->value);
                    break;
                case Json::Kind::string:
                    fields->saveField(key, val->text);
                    break;
                case Json::Kind::array: {
                    int index = 0;
                    for (auto& element : val->sequence) {
                        auto indexedKey = key + strIndex(index);
                        switch (element->kind) {
                            case Json::Kind::null:
                            case Json::Kind::object:
                            case Json::Kind::array:
                                halt("invalid type for field \"" + indexedKey + "\"");
                                break;
                            case Json::Kind::boolean:
                                fields->saveField(key, (element->value != 0));
                                break;
                            case Json::Kind::number:
                                fields->saveField(key, element->value);
                                break;
                            case Json::Kind::string:
                                fields->saveField(key, element->text);
                                break;
                        }
                        ++index;
                    }
                    break;
                }
            }
        }
    };

    try {
        // parse JSON
        std::size_t pos = 0;
        auto lookahead = lex(src, pos);
        Json js = std::move(json(src, pos, lookahead));
        check(Token::Kind::eof, lookahead); // ensures the entire input was processed

        // now we interpret it
        if (js.kind != Json::Kind::object) halt("expected an object at the toplevel");
        auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
        for (auto& metaobj : js.fields) {
            auto metatype = metaobj.first;
            auto& payload = metaobj.second;
            if (metatype != "Components") {
                fields->clear();
                json2fields(fields.get(), metatype, *payload);
                put(metatype, metatype, 0, fields.get());
            } else {
                if (payload->kind != Json::Kind::object) halt("expected \"Components\" to be an object");
                for (auto& obj : payload->fields) {
                    auto name = obj.first;
                    auto& contents = obj.second;
                    fields->clear();
                    json2fields(fields.get(), name, *contents);
                    auto type = fields->loadField("typename");
                    Util::identification id = fields->loadField("id", -1);
                    put(name, type, id, fields.get());
                }
            }
        }
    } catch(const std::runtime_error& e) {
        _model->getTracer()->traceError(e.what());
        return false;
    }

    return true;
}


bool JsonSerializer::get(const std::string& name, PersistenceRecord *entry) {
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


bool JsonSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
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


int JsonSerializer::for_each(std::function<int(const std::string&)> delegate) {
    // enfore id-order
    std::vector<std::pair<bool, std::string>> sorted;
    sorted.reserve(_metaobjects.size() + _components.size());
    for (auto& entry : _metaobjects) sorted.push_back({true, entry.first});
    for (auto& entry : _components) sorted.push_back({false, entry.first});
    std::sort(sorted.begin(), sorted.end(), [&](auto& a, auto& b){
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
