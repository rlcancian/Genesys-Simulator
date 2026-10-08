#include <gtest/gtest.h>

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/persistence/GenSerializer.h"

#include <memory>
#include <sstream>
#include <string>

namespace {

std::string complexTextPayload() {
    return
        "include gro;\n"
        "set ( \"dt\", 0.18 );\n"
        "set ( \"population_max\", 256 );\n"
        "\n"
        "\tprogram p() := {\n"
        "  selected : { message ( 1, tostring ( volume ) ) }\n"
        "  // source comment that belongs to the Gro payload\n"
        "  path := \"C:\\\\genesys\\\\gro\";\n"
        "  equation := \"x=y\";\n"
        "};\n";
}

std::unique_ptr<PersistenceRecord> makeGroProgramRecord(GenSerializer& serializer,
                                                        const std::string& name,
                                                        const std::string& sourceCode) {
    auto fields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
    fields->saveField("id", 143u);
    fields->saveField("typename", "GroProgram");
    fields->saveField("name", name);
    fields->saveField("sourceCode", sourceCode);
    return fields;
}

} // namespace

TEST(GenSerializerTextPersistence, ComplexTextFieldRoundTripsExactly) {
    Simulator savingSimulator;
    ASSERT_NE(savingSimulator.getPluginManager(), nullptr);
    savingSimulator.getPluginManager()->autoInsertPlugins();

    Model* savingModel = savingSimulator.getModelManager()->newModel();
    ASSERT_NE(savingModel, nullptr);

    const std::string sourceCode = complexTextPayload();
    GenSerializer writer(savingModel);
    auto fields = makeGroProgramRecord(writer, "ComplexGro", sourceCode);
    ASSERT_TRUE(writer.put("ComplexGro", "GroProgram", 143u, fields.get()));

    std::ostringstream serialized;
    ASSERT_TRUE(writer.dump(serialized));

    Simulator loadingSimulator;
    ASSERT_NE(loadingSimulator.getPluginManager(), nullptr);
    loadingSimulator.getPluginManager()->autoInsertPlugins();

    Model* loadingModel = loadingSimulator.getModelManager()->newModel();
    ASSERT_NE(loadingModel, nullptr);

    GenSerializer reader(loadingModel);
    std::istringstream input(serialized.str());
    ASSERT_TRUE(reader.load(input));

    auto loadedFields = std::unique_ptr<PersistenceRecord>(reader.newPersistenceRecord());
    ASSERT_TRUE(reader.get("ComplexGro", loadedFields.get()));
    EXPECT_EQ(loadedFields->loadField("sourceCode", std::string{}), sourceCode);
}
