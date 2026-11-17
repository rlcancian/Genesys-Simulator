#ifndef GUIEXTENSIONDEPENDENCYRESOLVER_H
#define GUIEXTENSIONDEPENDENCYRESOLVER_H

#include "GuiExtensionContracts.h"

#include <algorithm>
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

        std::unordered_set<std::string> models, capabilities;
        for (const auto& s : env.modelPluginIds) models.insert(normalizeId(s, true));
        for (const auto& s : env.systemCapabilityIds) capabilities.insert(normalizeId(s));

        enum class Visit { Visiting, Active, Inactive };
        std::unordered_map<std::string, Visit> states;
        std::function<bool(const GuiExtensionPlugin*)> visit;
        visit = [&](const GuiExtensionPlugin* plugin) -> bool {
            const auto id = normalizeId(plugin->extensionId());
            auto current = states.find(id);
            if (current != states.end()) {
                if (current->second == Visit::Visiting) {
                    result.issues.push_back({IssueCode::DependencyCycle, id, "cyclic GUI-extension dependency"});
                    return false;
                }
                return current->second == Visit::Active;
            }
            states.emplace(id, Visit::Visiting);
            bool valid = true;
            for (const auto& model : plugin->requiredModelPlugins()) {
                const auto key = normalizeId(model, true);
                if (env.modelPluginIdsKnown && !key.empty() && !models.contains(key)) {
                    result.issues.push_back({IssueCode::MissingDependency, id, "model:" + key});
                    valid = false;
                }
            }
            for (const auto& group : plugin->dependencyGroups()) {
                if (!group.required) continue;  // Optional requirements do not disable a plugin.
                if (group.ids.empty()) {
                    result.issues.push_back({IssueCode::MissingDependency, id, "empty dependency group"});
                    valid = false;
                    continue;
                }
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
                        if (const auto it = byId.find(key); it != byId.end()) {
                            present = visit(it->second);
                        }
                        break;
                    }
                    any |= present;
                    all &= present;
                }
                if (!(group.mode == GuiExtensionDependencyMode::All ? all : any)) {
                    result.issues.push_back({IssueCode::MissingDependency, id, "group requirement not satisfied"});
                    valid = false;
                }
            }
            states[id] = valid ? Visit::Active : Visit::Inactive;
            return valid;
        };
        for (const auto* plugin : ordered) {
            if (visit(plugin)) result.active.push_back(plugin);
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
                any |= found;
                all &= found;
            }
            if (!(group.mode == GuiExtensionDependencyMode::All ? all : any)) return false;
        }
        return true;
    }
};

#endif // GUIEXTENSIONDEPENDENCYRESOLVER_H
