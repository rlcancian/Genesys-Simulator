#include <gtest/gtest.h>

#include "kernel/simulator/Persistence.h"

class FakeModelPersistenceB : public ModelPersistence_if {
public:
    bool save(std::string) override { return false; }
    bool load(std::string) override { return false; }
    bool hasChanged() override { return false; }
    bool getOption(ModelPersistence_if::Options) override { return false; }
    void setOption(ModelPersistence_if::Options, bool) override {}
    std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

TEST(SupportPersistenceClassTest, InsertGetAndEraseField) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    fields.saveField("a", std::string("1"));
    ASSERT_EQ(fields.size(), 1u);
    EXPECT_EQ(fields.loadField("a", std::string("")), "1");

    fields.erase("a");
    EXPECT_EQ(fields.size(), 0u);
}

TEST(SupportPersistenceClassTest, MissingFieldReturnsEmptyString) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    EXPECT_EQ(fields.loadField("missing", std::string("")), "");
}

TEST(SupportPersistenceClassTest, SaveFieldSkipsStringDefaultUnlessForced) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    fields.saveField("name", std::string("default"), std::string("default"), false);
    EXPECT_EQ(fields.find("name"), fields.end());

    fields.saveField("name", std::string("default"), std::string("default"), true);
    ASSERT_NE(fields.find("name"), fields.end());
    EXPECT_EQ(fields.loadField("name", std::string("")), "default");
}

TEST(SupportPersistenceClassTest, SaveFieldSkipsNumericDefaultUnlessForced) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    fields.saveField("double", 2.5, 2.5, false);
    fields.saveField("unsigned", static_cast<unsigned int>(3), static_cast<unsigned int>(3), false);
    fields.saveField("integer", 4, 4, false);
    fields.saveField("time", Util::TimeUnit::second, Util::TimeUnit::second, false);
    EXPECT_EQ(fields.size(), 0u);

    fields.saveField("double", 2.5, 2.5, true);
    fields.saveField("unsigned", static_cast<unsigned int>(3), static_cast<unsigned int>(3), true);
    fields.saveField("integer", 4, 4, true);
    fields.saveField("time", Util::TimeUnit::second, Util::TimeUnit::second, true);
    EXPECT_EQ(fields.size(), 4u);
}

TEST(SupportPersistenceClassTest, LoadFieldNumericOverloadsReadStoredValues) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    fields.saveField("double", 1.25, 0.0, true);
    fields.saveField("unsigned", static_cast<unsigned int>(17));
    fields.saveField("integer", -8, 0, true);
    fields.saveField("time", Util::TimeUnit::minute, Util::TimeUnit::second, true);

    EXPECT_DOUBLE_EQ(fields.loadField("double", 0.0), 1.25);
    EXPECT_EQ(fields.loadField("unsigned", static_cast<unsigned int>(0)), 17u);
    EXPECT_EQ(fields.loadField("integer", 0), -8);
    EXPECT_EQ(fields.loadField("time", Util::TimeUnit::second), Util::TimeUnit::minute);
}

TEST(SupportPersistenceClassTest, ClearAndFindReflectContainerState) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);

    fields.saveField("a", std::string("1"));
    fields.saveField("b", std::string("2"));
    ASSERT_NE(fields.find("a"), fields.end());
    ASSERT_EQ(fields.find("missing"), fields.end());

    fields.clear();
    EXPECT_EQ(fields.size(), 0u);
    EXPECT_EQ(fields.find("a"), fields.end());
    EXPECT_EQ(fields.find("b"), fields.end());
}

TEST(SupportPersistenceClassTest, NewInstanceReturnsIndependentRecord) {
    FakeModelPersistenceB persistence;
    PersistenceRecord fields(persistence);
    fields.saveField("a", std::string("main"));

    PersistenceRecord* clone = fields.newInstance();
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->size(), 0u);

    clone->saveField("b", std::string("clone"));
    EXPECT_EQ(fields.find("b"), fields.end());
    EXPECT_EQ(fields.size(), 1u);
    EXPECT_EQ(clone->size(), 1u);

    delete clone;
}

TEST(SupportPersistenceClassTest, InsertRangeCopiesRawPayloadEntries) {
    FakeModelPersistenceB persistence;
    PersistenceRecord source(persistence);
    PersistenceRecord target(persistence);

    source.saveField("alpha", std::string("text"));
    source.saveField("beta", 12u);

    target.insert(source.begin(), source.end());

    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(target.loadField("alpha", std::string("")), "text");
    EXPECT_EQ(target.loadField("beta", static_cast<unsigned int>(0)), 12u);
}
