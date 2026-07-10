#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/ModelManager.h"
#include "plugins/PluginConnectorDummyImpl1.h"
#include "plugins/components/AI/AIAssistant.h"
#include "tools/AIAssistant/AIConversationService.h"

#include <memory>

namespace {
class FakeProviderClient : public AIProviderClient_if {
public:
    std::string getProviderName() const override { return "Fake"; }
    bool isAvailable() const override { return true; }
    void setBaseUrl(const std::string& value) override { baseUrl = value; }
    std::string getBaseUrl() const override { return baseUrl; }
    void setTimeoutSeconds(unsigned int value) override { timeout = value; }
    unsigned int getTimeoutSeconds() const override { return timeout; }
    void setApiKey(const std::string& value) override { hasKey = !value.empty(); }
    void clearApiKey() override { hasKey = false; }
    bool hasApiKey() const override { return hasKey; }
    ProviderResponse send(const ProviderRequest& request) override {
        requests.push_back(request);
        return {true, "{\"action\":\"route\"}", "", 200, 7, 3};
    }

    std::string baseUrl;
    unsigned int timeout = 0;
    bool hasKey = false;
    std::vector<ProviderRequest> requests;
};
}

TEST(AIConversationServiceTest, KeepsIndependentBoundedHistories) {
    AIConversationService service;
    AIConversationConfiguration configuration;
    configuration.systemInstructions = "Return JSON";
    configuration.model = "fake-model";
    configuration.maxHistoryMessages = 2;
    service.setConfiguration(configuration);

    auto fake = std::make_unique<FakeProviderClient>();
    FakeProviderClient* observer = fake.get();
    service.setProviderClient(std::move(fake));

    ASSERT_TRUE(service.send("entity-1", "first").success);
    ASSERT_TRUE(service.send("entity-1", "second").success);
    ASSERT_TRUE(service.send("entity-2", "other").success);

    ASSERT_EQ(observer->requests.size(), 3u);
    EXPECT_EQ(observer->requests[0].messages.size(), 2u);
    EXPECT_EQ(observer->requests[1].messages.size(), 4u);
    EXPECT_EQ(observer->requests[1].messages[1].content, "first");
    EXPECT_EQ(observer->requests[2].messages.size(), 2u);
    EXPECT_EQ(observer->requests[2].messages.back().content, "other");
}

TEST(AIPluginTest, BuiltInConnectorExposesSupportAndComponentMetadata) {
    PluginConnectorDummyImpl1 connector;
    std::unique_ptr<Plugin> support(connector.connect("aisupport.so"));
    std::unique_ptr<Plugin> assistant(connector.connect("aiassistant.so"));

    ASSERT_NE(support, nullptr);
    ASSERT_NE(assistant, nullptr);
    EXPECT_EQ(support->getPluginInfo()->getCategory(), "AI");
    EXPECT_EQ(assistant->getPluginInfo()->getCategory(), "AI");
    EXPECT_EQ(assistant->getPluginInfo()->getMinimumInputs(), 1u);
    EXPECT_EQ(assistant->getPluginInfo()->getMaximumOutputs(), 999u);
}

TEST(AIPluginTest, PromptTemplateEvaluatesExpressionsAndEscapesLiteralBraces) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    AIAssistant assistant(model, "Decision");
    assistant.setPromptTemplate("time={1 + 2}; json={{\"action\":\"route\"}}");

    std::string prompt;
    std::string error;
    ASSERT_TRUE(assistant.renderPrompt(prompt, error)) << error;
    EXPECT_EQ(prompt, "time=3; json={\"action\":\"route\"}");
}
