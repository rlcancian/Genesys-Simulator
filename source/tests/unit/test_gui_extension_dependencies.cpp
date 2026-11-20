#include "extensions/GuiExtensionDependencyResolver.h"
#include "extensions/GuiExtensionManager.h"
#include "extensions/GuiExtensionPluginCatalog.h"

#include <gtest/gtest.h>

#include <QAction>
#include <QMainWindow>
#include <QByteArray>
#include <QTemporaryDir>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

using Resolver = GuiExtensionDependencyResolver;
using Group = GuiExtensionDependencyGroup;
using Kind = GuiExtensionDependencyKind;
using Mode = GuiExtensionDependencyMode;

class DependencyFixturePlugin final : public GuiExtensionPlugin {
public:
    explicit DependencyFixturePlugin(std::string id,
                                     std::vector<std::string> legacy = {},
                                     GuiExtensionDependencyGroups groups = {})
        : id_(std::move(id)), legacy_(std::move(legacy)), groups_(std::move(groups)) {}

    std::string extensionId() const override { return id_; }
    std::string displayName() const override { return id_; }
    std::string version() const override { return "1.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return legacy_; }
    GuiExtensionDependencyGroups dependencyGroups() const override { return groups_; }
    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (!registry) return;
        GuiActionContribution action;
        action.id = id_;
        registry->addAction(std::move(action));
    }

private:
    std::string id_;
    std::vector<std::string> legacy_;
    GuiExtensionDependencyGroups groups_;
};

Group group(Kind kind, Mode mode, std::vector<std::string> ids, bool required = true) {
    return {kind, mode, std::move(ids), required};
}

bool hasIssue(const Resolver::Result& result, Resolver::IssueCode code) {
    return std::any_of(result.issues.begin(), result.issues.end(),
                       [code](const auto& issue) { return issue.code == code; });
}

TEST(GuiExtensionDependencyF1, LegacyRequirementsRemainAnd) {
    DependencyFixturePlugin plugin("queue.visualization", {"Queue", "PickStation"});
    auto result = Resolver::resolve({&plugin}, {{"queue"}, {}});
    EXPECT_TRUE(result.active.empty());
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::MissingDependency));

    result = Resolver::resolve({&plugin}, {{"QUEUE.so", " PickStation "}, {}});
    ASSERT_EQ(result.active.size(), 1u);
    EXPECT_EQ(result.active.front(), &plugin);
}

TEST(GuiExtensionDependencyF1, AnyGroupAcceptsOnePresentModel) {
    DependencyFixturePlugin plugin("any", {}, {group(Kind::ModelPlugin, Mode::Any, {"Queue", "PickStation"})});
    const auto one = Resolver::resolve({&plugin}, {{"Queue"}, {}});
    EXPECT_TRUE(one.contains(&plugin));
    const auto none = Resolver::resolve({&plugin}, {});
    EXPECT_FALSE(none.contains(&plugin));
}

TEST(GuiExtensionDependencyF1, AllGroupRequiresAllModelPlugins) {
    DependencyFixturePlugin plugin("all", {}, {group(Kind::ModelPlugin, Mode::All, {"Queue", "PickStation"})});
    EXPECT_FALSE(Resolver::resolve({&plugin}, {{"Queue"}, {}}).contains(&plugin));
    EXPECT_TRUE(Resolver::resolve({&plugin}, {{"Queue", "PickStation"}, {}}).contains(&plugin));
}

TEST(GuiExtensionDependencyF1, OptionalRequirementsNeverDisableHost) {
    DependencyFixturePlugin plugin("optional", {}, {
        group(Kind::GuiExtension, Mode::All, {"not-installed"}, false),
        group(Kind::SystemCapability, Mode::All, {"gpu"}, false)});
    EXPECT_TRUE(Resolver::resolve({&plugin}, {}).contains(&plugin));
}

TEST(GuiExtensionDependencyF1, RequiredSystemCapabilityBlocks) {
    DependencyFixturePlugin plugin("cap", {}, {group(Kind::SystemCapability, Mode::All, {"cuda"})});
    EXPECT_FALSE(Resolver::resolve({&plugin}, {}).contains(&plugin));
    EXPECT_TRUE(Resolver::resolve({&plugin}, {{}, {"CUDA"}}).contains(&plugin));
}

TEST(GuiExtensionDependencyF1, TransitiveGuiExtensionRequirements) {
    DependencyFixturePlugin a("a"), b("b", {}, {group(Kind::GuiExtension, Mode::All, {"a"})});
    DependencyFixturePlugin c("c", {}, {group(Kind::GuiExtension, Mode::All, {"b"})});
    const auto result = Resolver::resolve({&c, &b, &a}, {});
    EXPECT_TRUE(result.contains(&a));
    EXPECT_TRUE(result.contains(&b));
    EXPECT_TRUE(result.contains(&c));
}

TEST(GuiExtensionDependencyF1, NonCyclicDependentIsNotMisdiagnosedAsCycle) {
    DependencyFixturePlugin a("a", {}, {group(Kind::GuiExtension, Mode::All, {"b"})});
    DependencyFixturePlugin b("b", {}, {group(Kind::GuiExtension, Mode::All, {"c"})});
    DependencyFixturePlugin c("c", {}, {group(Kind::GuiExtension, Mode::All, {"b"})});
    const auto result = Resolver::resolve({&a, &b, &c}, {});
    EXPECT_TRUE(result.active.empty());
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::DependencyCycle));
    for (const auto& issue : result.issues) {
        if (issue.code == Resolver::IssueCode::DependencyCycle) {
            EXPECT_NE(issue.extensionId, "a");
        }
    }
}

TEST(GuiExtensionDependencyF1, MissingRequiredGuiExtensionFails) {
    DependencyFixturePlugin a("a", {}, {group(Kind::GuiExtension, Mode::All, {"missing"})});
    auto result = Resolver::resolve({&a}, {});
    EXPECT_FALSE(result.contains(&a));
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::MissingDependency));
}

TEST(GuiExtensionDependencyF1, CycleIsRejectedAndDiagnosed) {
    DependencyFixturePlugin a("a", {}, {group(Kind::GuiExtension, Mode::All, {"b"})});
    DependencyFixturePlugin b("b", {}, {group(Kind::GuiExtension, Mode::All, {"a"})});
    const auto result = Resolver::resolve({&a, &b}, {});
    EXPECT_TRUE(result.active.empty());
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::DependencyCycle));
}

TEST(GuiExtensionDependencyF1, ConflictingIdsRejectedDeterministically) {
    DependencyFixturePlugin first("GUI.Example"), duplicate(" gui.example ");
    auto result = Resolver::resolve({&first, &duplicate}, {});
    ASSERT_EQ(result.active.size(), 1u);
    EXPECT_EQ(result.active.front(), &first);
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::DuplicateId));
}

TEST(GuiExtensionDependencyF1, EmptyIdRejected) {
    DependencyFixturePlugin plugin(" \t ");
    const auto result = Resolver::resolve({&plugin}, {});
    EXPECT_TRUE(result.active.empty());
    EXPECT_TRUE(hasIssue(result, Resolver::IssueCode::EmptyId));
}

TEST(GuiExtensionDependencyF1, ContributionEligibilityIsIndependent) {
    DependencyFixturePlugin plugin("host");
    const auto active = Resolver::resolve({&plugin}, {{"Queue"}, {}});
    GuiExtensionRegistry declared, available;
    GuiActionContribution common;
    common.id = "common";
    declared.addAction(common);
    GuiActionContribution optional;
    optional.id = "queue";
    optional.eligibility = {group(Kind::ModelPlugin, Mode::All, {"Queue"})};
    declared.addAction(optional);
    GuiActionContribution absent;
    absent.id = "pick";
    absent.eligibility = {group(Kind::ModelPlugin, Mode::All, {"PickStation"})};
    declared.addAction(absent);
    available.appendEligibleFrom(declared, [&](const auto& groups) {
        return Resolver::contributionEligible(groups, {{"Queue"}, {}}, active);
    });
    ASSERT_EQ(available.actions().size(), 2u);
    EXPECT_EQ(available.actions()[0].id, "common");
    EXPECT_EQ(available.actions()[1].id, "queue");
}

TEST(GuiExtensionDependencyF1, GuiContributionDependsOnActiveGuiExtension) {
    DependencyFixturePlugin a("a"), b("b", {"not-loaded"});
    const auto active = Resolver::resolve({&a, &b}, {});
    const auto requirement = GuiExtensionDependencyGroups{
        group(Kind::GuiExtension, Mode::All, {"b"})};
    EXPECT_FALSE(Resolver::contributionEligible(requirement, {}, active));
    EXPECT_TRUE(Resolver::contributionEligible(
        {group(Kind::GuiExtension, Mode::All, {"a"})}, {}, active));
}

TEST(GuiExtensionDependencyF1, EmptyRequiredGroupIsInvalid) {
    DependencyFixturePlugin a("a", {}, {group(Kind::ModelPlugin, Mode::Any, {})});
    EXPECT_FALSE(Resolver::resolve({&a}, {}).contains(&a));
}

TEST(GuiExtensionDependencyF1, ExistingPluginRegistrationMethodStillWorks) {
    DependencyFixturePlugin plugin("compat");
    GuiExtensionRegistry registry;
    plugin.registerContributions(&registry);
    ASSERT_EQ(registry.actions().size(), 1u);
    EXPECT_EQ(registry.actions()[0].id, "compat");
}

TEST(GuiExtensionDependencyF1, UnknownLegacyInventoryKeepsPreviouslyRegisteredExtensions) {
    DependencyFixturePlugin legacy("legacy", {"Queue"});
    Resolver::Environment unknown;
    unknown.modelPluginIdsKnown = false;
    EXPECT_TRUE(Resolver::resolve({&legacy}, unknown).contains(&legacy));
    EXPECT_FALSE(Resolver::resolve({&legacy}, {}).contains(&legacy));
}

TEST(GuiExtensionDependencyF1, GuiActivationOrderRespectsTransitiveRequirements) {
    DependencyFixturePlugin a("a");
    DependencyFixturePlugin b("b", {}, {group(Kind::GuiExtension, Mode::All, {"a"})});
    DependencyFixturePlugin c("c", {}, {group(Kind::GuiExtension, Mode::All, {"b"})});
    const auto resolved = Resolver::resolve({&c, &b, &a}, {});
    ASSERT_EQ(resolved.active.size(), 3u);
    EXPECT_EQ(resolved.active[0], &a);
    EXPECT_EQ(resolved.active[1], &b);
    EXPECT_EQ(resolved.active[2], &c);
}

TEST(GuiExtensionDependencyF1, AnyCycleWithIndependentRootCanActivate) {
    DependencyFixturePlugin root("root");
    DependencyFixturePlugin a("a", {}, {
        group(Kind::GuiExtension, Mode::Any, {"b", "root"})});
    DependencyFixturePlugin b("b", {}, {
        group(Kind::GuiExtension, Mode::All, {"a"})});
    const auto resolved = Resolver::resolve({&b, &a, &root}, {});
    EXPECT_TRUE(resolved.contains(&root));
    EXPECT_TRUE(resolved.contains(&a));
    EXPECT_TRUE(resolved.contains(&b));
    EXPECT_FALSE(hasIssue(resolved, Resolver::IssueCode::DependencyCycle));
}

TEST(GuiExtensionDependencyF1, ManagerUsesSharedResolverForActionRegistration) {
    class ActionPlugin final : public GuiExtensionPlugin {
    public:
        ActionPlugin(std::string id, GuiExtensionDependencyGroups groups)
            : id_(std::move(id)), groups_(std::move(groups)) {}
        std::string extensionId() const override { return id_; }
        std::string displayName() const override { return id_; }
        std::string version() const override { return "1.0.0"; }
        std::vector<std::string> requiredModelPlugins() const override { return {}; }
        GuiExtensionDependencyGroups dependencyGroups() const override { return groups_; }
        void registerContributions(GuiExtensionRegistry* registry) const override {
            GuiActionContribution action;
            action.id = id_;
            action.text = id_;
            action.menuPath = "TestExtensions";
            action.trigger = [](const GuiExtensionRuntimeContext&) {};
            registry->addAction(std::move(action));
        }
    private:
        std::string id_;
        GuiExtensionDependencyGroups groups_;
    };
    ActionPlugin normal("normal", {});
    ActionPlugin unavailable("unavailable",
        {group(Kind::ModelPlugin, Mode::All, {"PickStation"})});
    QMainWindow window;
    GuiExtensionManager manager(&window);
    manager.setPlugins({&normal, &unavailable});
    manager.setLoadedModelPluginIds({"Queue"});
    GuiExtensionRuntimeContext context;
    context.mainWindow = &window;
    manager.rebuild(context);
    EXPECT_NE(window.findChild<QAction*>("normal"), nullptr);
    EXPECT_EQ(window.findChild<QAction*>("unavailable"), nullptr);
    manager.setLoadedModelPluginIds({"Queue", "PickStation"});
    manager.rebuild(context);
    EXPECT_NE(window.findChild<QAction*>("unavailable"), nullptr);
}

TEST(GuiExtensionDependencyF1, NumericDottedMinimumVersionIsStrict) {
    EXPECT_TRUE(Resolver::versionAtLeast("1.2.0", "1.1.9"));
    EXPECT_TRUE(Resolver::versionAtLeast("1.0", "1.0.0"));
    EXPECT_FALSE(Resolver::versionAtLeast("1.0.9", "1.1"));
    EXPECT_FALSE(Resolver::versionAtLeast("1.0-beta", "1.0"));
    EXPECT_FALSE(Resolver::versionAtLeast("", "1.0"));
    EXPECT_FALSE(Resolver::versionAtLeast("1.0", "1.0."));
}

TEST(GuiExtensionDependencyF1, ModelVersionMustBeKnownAndCompatible) {
    auto rule = group(Kind::ModelPlugin, Mode::All, {"Queue"});
    rule.minimumVersion = "1.2.0";
    DependencyFixturePlugin plugin("versioned", {}, {rule});
    Resolver::Environment env;
    env.modelPluginIds = {"Queue"};
    EXPECT_FALSE(Resolver::resolve({&plugin}, env).contains(&plugin));
    env.modelPluginVersions = {{"Queue", "1.1.9"}};
    EXPECT_FALSE(Resolver::resolve({&plugin}, env).contains(&plugin));
    env.modelPluginVersions = {{" queue ", "1.2.0"}};
    EXPECT_TRUE(Resolver::resolve({&plugin}, env).contains(&plugin));
}

TEST(GuiExtensionDependencyF1, GuiVersionCompatibilityGatesDependentExtension) {
    DependencyFixturePlugin provider("provider");
    auto rule = group(Kind::GuiExtension, Mode::All, {"provider"});
    rule.minimumVersion = "2.0.0";
    DependencyFixturePlugin consumer("consumer", {}, {rule});
    EXPECT_FALSE(Resolver::resolve({&consumer, &provider}, {}).contains(&consumer));
    rule.minimumVersion = "1.0.0";
    DependencyFixturePlugin compatible("compatible", {}, {rule});
    EXPECT_TRUE(Resolver::resolve({&compatible, &provider}, {}).contains(&compatible));
}

TEST(GuiExtensionDependencyF1, SystemCapabilityVersionRequiresMetadata) {
    auto rule = group(Kind::SystemCapability, Mode::All, {"gpu"});
    rule.minimumVersion = "2.0";
    DependencyFixturePlugin plugin("gpu-visualizer", {}, {rule});
    Resolver::Environment env;
    env.systemCapabilityIds = {"gpu"};
    EXPECT_FALSE(Resolver::resolve({&plugin}, env).contains(&plugin));
    env.systemCapabilityVersions = {{"GPU", "2.1.0"}};
    EXPECT_TRUE(Resolver::resolve({&plugin}, env).contains(&plugin));
}

TEST(GuiExtensionDependencyF1, StaticCatalogAppliesSameModelPluginAndPolicy) {
    // Exercise the existing real compiled Queue/PickStation GUI extension,
    // without adding temporary pointers to the process-global plugin registry.
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto previous = qgetenv("GENESYS_GUI_EXTENSIONS_CONFIG");
    const bool hadPrevious = qEnvironmentVariableIsSet("GENESYS_GUI_EXTENSIONS_CONFIG");
    qputenv("GENESYS_GUI_EXTENSIONS_CONFIG",
            (directory.path() + "/no-settings.json").toUtf8());
    const auto active = [] {
        for (const auto* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
            if (plugin && plugin->extensionId() == "discrete-processing.visualization.gui") return true;
        }
        return false;
    };
    GuiExtensionPluginCatalog::setAvailableModelPluginIds({"Queue"});
    EXPECT_FALSE(active());
    GuiExtensionPluginCatalog::setAvailableModelPluginIds({"Queue", "PickStation"});
    EXPECT_TRUE(active());
    GuiExtensionPluginCatalog::clearAvailableModelPluginInventory();
    if (hadPrevious) qputenv("GENESYS_GUI_EXTENSIONS_CONFIG", previous);
    else qunsetenv("GENESYS_GUI_EXTENSIONS_CONFIG");
}

} // namespace
