#include "extensions/GuiExtensionDependencyResolver.h"

#include <gtest/gtest.h>

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

} // namespace
