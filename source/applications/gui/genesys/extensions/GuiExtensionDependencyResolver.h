#ifndef GUIEXTENSIONDEPENDENCYRESOLVER_H
#define GUIEXTENSIONDEPENDENCYRESOLVER_H

#include "GuiExtensionContracts.h"

#include <algorithm>
#include <charconv>
#include <map>
#include <cctype>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Application-local static GUI-extension dependency policy. No kernel or Qt runtime
// object is created, owned or modified during resolution.
class GuiExtensionDependencyResolver {
public:
    struct Environment {
        std::vector<std::string> modelPluginIds;
        std::vector<std::string> systemCapabilityIds;
        // False only for legacy catalog calls before the GUI supplies an inventory.
        // This is UNKNOWN, not a claim that the plugin is installed.
        bool modelPluginIdsKnown = true;
        // Versions are optional until a dependency explicitly requests one.
        std::unordered_map<std::string, std::string> modelPluginVersions;
        std::unordered_map<std::string, std::string> systemCapabilityVersions;
    };

    enum class IssueCode { EmptyId, DuplicateId, MissingDependency, DependencyCycle };
    struct Issue {
        IssueCode code;
        std::string extensionId;
        std::string detail;
    };
    struct Result {
        std::vector<const GuiExtensionPlugin*> active;
        std::vector<Issue> issues;

        bool contains(const GuiExtensionPlugin* plugin) const {
            return std::find(active.begin(), active.end(), plugin) != active.end();
        }
    };

    static std::string normalizeId(const std::string& text, bool modelPlugin = false) {
        std::string out;
        out.reserve(text.size());
        for (unsigned char ch : text) {
            if (!std::isspace(ch)) out.push_back(static_cast<char>(std::tolower(ch)));
        }
        if (modelPlugin && out.size() > 3 && out.ends_with(".so")) {
            out.resize(out.size() - 3);
        }
        return out;
    }

    // Intentionally restricted to numeric dotted versions. Unknown/pre-release
    // grammars fail closed instead of silently passing a compatibility check.
    static bool versionAtLeast(const std::string& actual, const std::string& minimum) {
        const auto parse = [](const std::string& value, std::vector<unsigned int>* parts) {
            if (value.empty()) return false;
            std::size_t start = 0;
            while (start < value.size()) {
                const std::size_t dot = value.find('.', start);
                const std::size_t end = dot == std::string::npos ? value.size() : dot;
                if (end == start) return false;
                unsigned int number = 0;
                const char* first = value.data() + start;
                const char* last = value.data() + end;
                const auto result = std::from_chars(first, last, number);
                if (result.ec != std::errc{} || result.ptr != last) return false;
                parts->push_back(number);
                if (dot == std::string::npos) return true;
                start = dot + 1;
            }
            return false;
        };
        std::vector<unsigned int> a, b;
        if (!parse(actual, &a) || !parse(minimum, &b)) return false;
        const auto size = std::max(a.size(), b.size());
        a.resize(size, 0);
        b.resize(size, 0);
        return a >= b;
    }

    // The input plugins are already enabled/disabled filtered by the catalog.
    // The same function must be used for every Manager and Catalog dispatch.
    static Result resolve(const std::vector<const GuiExtensionPlugin*>& candidates,
                          const Environment& env) {
        Result result;
        std::unordered_map<std::string, const GuiExtensionPlugin*> byId;
        std::vector<const GuiExtensionPlugin*> ordered;
        for (const auto* plugin : candidates) {
            if (!plugin) continue;
            const auto id = normalizeId(plugin->extensionId());
            if (id.empty()) {
                result.issues.push_back({IssueCode::EmptyId, "", "empty graphical extension id"});
                continue;
            }
            if (!byId.emplace(id, plugin).second) {
                result.issues.push_back({IssueCode::DuplicateId, id, "duplicate graphical extension id"});
                continue;
            }
            ordered.push_back(plugin);
        }

        std::unordered_set<std::string> models, capabilities, activeIds;
        std::unordered_map<std::string, std::string> modelVersions, systemVersions;
        for (const auto& [key, value] : env.modelPluginVersions)
            modelVersions[normalizeId(key, true)] = value;
        for (const auto& [key, value] : env.systemCapabilityVersions)
            systemVersions[normalizeId(key)] = value;
        for (const auto& s : env.modelPluginIds) models.insert(normalizeId(s, true));
        for (const auto& s : env.systemCapabilityIds) capabilities.insert(normalizeId(s));

        const auto groupSatisfied = [&](const GuiExtensionDependencyGroup& group) {
            if (!group.required) return true;
            if (group.ids.empty()) return false;
            bool any = false, all = true;
            for (const auto& dependency : group.ids) {
                const auto key = normalizeId(dependency, group.kind == GuiExtensionDependencyKind::ModelPlugin);
                bool present = false;
                switch (group.kind) {
                case GuiExtensionDependencyKind::ModelPlugin:
                    present = models.contains(key);
                    break;
                case GuiExtensionDependencyKind::SystemCapability:
                    present = capabilities.contains(key);
                    break;
                case GuiExtensionDependencyKind::GuiExtension:
                    present = activeIds.contains(key);
                    break;
                }
                if (present && !group.minimumVersion.empty()) {
                    std::string actualVersion;
                    if (group.kind == GuiExtensionDependencyKind::ModelPlugin) {
                        if (auto it = modelVersions.find(key); it != modelVersions.end()) actualVersion = it->second;
                    } else if (group.kind == GuiExtensionDependencyKind::SystemCapability) {
                        if (auto it = systemVersions.find(key); it != systemVersions.end()) actualVersion = it->second;
                    } else if (auto it = byId.find(key); it != byId.end()) {
                        actualVersion = it->second->version();
                    }
                    present = versionAtLeast(actualVersion, group.minimumVersion);
                }
                any |= present;
                all &= present;
            }
            return group.mode == GuiExtensionDependencyMode::All ? all : any;
        };

        const auto eligible = [&](const GuiExtensionPlugin* plugin) {
            // Existing requiredModelPlugins() has strict ALL semantics when
            // the model-plugin inventory is known. Unknown is not the same as
            // an authoritative empty loaded-plugin inventory.
            if (env.modelPluginIdsKnown) {
                for (const auto& model : plugin->requiredModelPlugins()) {
                    const auto key = normalizeId(model, true);
                    if (!key.empty() && !models.contains(key)) return false;
                }
            }
            for (const auto& group : plugin->dependencyGroups()) {
                if (!groupSatisfied(group)) return false;
            }
            return true;
        };

        // Least fixed-point resolution: cyclic requirements cannot activate
        // solely by referring to one another. An ANY group with an independent
        // active alternative can activate and then unlock other dependents.
        // Append in dependency-first activation order without assuming that
        // static registration order is a topological order.
        bool changed = false;
        do {
            changed = false;
            for (const auto* plugin : ordered) {
                const auto id = normalizeId(plugin->extensionId());
                if (activeIds.contains(id) || !eligible(plugin)) continue;
                activeIds.insert(id);
                result.active.push_back(plugin);
                changed = true;
            }
        } while (changed);

        // Report the exact outstanding requirements for inactive extensions.
        // Cycles are detected only among inactive GUI dependencies.
        std::unordered_set<std::string> visited, visiting, cyclic;
        std::function<bool(const std::string&)> hasCycle;
        hasCycle = [&](const std::string& id) -> bool {
            if (visiting.contains(id)) return true;
            if (visited.contains(id)) return cyclic.contains(id);
            visiting.insert(id);
            bool cycle = false;
            if (const auto it = byId.find(id); it != byId.end()) {
                for (const auto& group : it->second->dependencyGroups()) {
                    if (!group.required || group.kind != GuiExtensionDependencyKind::GuiExtension) continue;
                    for (const auto& name : group.ids) {
                        const auto next = normalizeId(name);
                        if (!activeIds.contains(next) && byId.contains(next)) {
                            cycle = hasCycle(next) || cycle;
                        }
                    }
                }
            }
            visiting.erase(id);
            visited.insert(id);
            if (cycle) cyclic.insert(id);
            return cycle;
        };
        for (const auto* plugin : ordered) {
            const auto id = normalizeId(plugin->extensionId());
            if (activeIds.contains(id)) continue;
            if (hasCycle(id)) {
                result.issues.push_back({IssueCode::DependencyCycle, id, "cyclic GUI-extension dependency"});
            }
            if (env.modelPluginIdsKnown) {
                for (const auto& name : plugin->requiredModelPlugins()) {
                    const auto key = normalizeId(name, true);
                    if (!key.empty() && !models.contains(key)) {
                        result.issues.push_back({IssueCode::MissingDependency, id, "model:" + key});
                    }
                }
            }
            for (const auto& group : plugin->dependencyGroups()) {
                if (group.required && !groupSatisfied(group)) {
                    result.issues.push_back({IssueCode::MissingDependency, id,
                                             "unmet required dependency group"});
                }
            }
        }
        return result;
    }

    // Optional dependency groups are informational: they never block unrelated
    // contribution categories. Required groups gate only their own contribution.
    static bool contributionEligible(const GuiExtensionDependencyGroups& groups,
                                     const Environment& env,
                                     const Result& resolved) {
        std::unordered_set<std::string> models, capabilities, extensions;
        for (const auto& id : env.modelPluginIds) models.insert(normalizeId(id, true));
        for (const auto& id : env.systemCapabilityIds) capabilities.insert(normalizeId(id));
        for (const auto* p : resolved.active) extensions.insert(normalizeId(p->extensionId()));
        for (const auto& group : groups) {
            if (!group.required) continue;
            if (group.ids.empty()) return false;
            bool any = false, all = true;
            for (const auto& name : group.ids) {
                const auto key = normalizeId(name, group.kind == GuiExtensionDependencyKind::ModelPlugin);
                bool found = false;
                switch (group.kind) {
                case GuiExtensionDependencyKind::ModelPlugin: found = models.contains(key); break;
                case GuiExtensionDependencyKind::GuiExtension: found = extensions.contains(key); break;
                case GuiExtensionDependencyKind::SystemCapability: found = capabilities.contains(key); break;
                }
                if (found && !group.minimumVersion.empty()) {
                    std::string actualVersion;
                    if (group.kind == GuiExtensionDependencyKind::ModelPlugin) {
                        for (const auto& [name, version] : env.modelPluginVersions)
                            if (normalizeId(name, true) == key) actualVersion = version;
                    } else if (group.kind == GuiExtensionDependencyKind::SystemCapability) {
                        for (const auto& [name, version] : env.systemCapabilityVersions)
                            if (normalizeId(name) == key) actualVersion = version;
                    } else {
                        for (const auto* plugin : resolved.active)
                            if (normalizeId(plugin->extensionId()) == key) actualVersion = plugin->version();
                    }
                    found = versionAtLeast(actualVersion, group.minimumVersion);
                }
                any |= found;
                all &= found;
            }
            if (!(group.mode == GuiExtensionDependencyMode::All ? all : any)) return false;
        }
        return true;
    }
};

#endif // GUIEXTENSIONDEPENDENCYRESOLVER_H
