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
