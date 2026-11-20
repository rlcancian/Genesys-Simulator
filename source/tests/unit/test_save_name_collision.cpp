// Regression test for the GUI model save/load bug investigation: GenSerializer
// and XmlSerializer key their save buffer purely by name across both data
// definitions and components (see GenSerializer::put()/XmlSerializer::put()),
// so two live objects that end up sharing a name silently overwrite each
// other in the saved file with no diagnostic. Both serializers now emit a
// traceError when this happens, turning a silent data-loss bug into a
// visible one.

#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/persistence/GenSerializer.h"
#include "kernel/simulator/persistence/XmlSerializer.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

namespace {

std::string g_lastErrorText;
int g_errorCount = 0;

void CaptureTraceError(TraceErrorEvent event) {
	g_lastErrorText = event.getText();
	++g_errorCount;
}

} // namespace

TEST(SaveNameCollisionTest, GenSerializerWarnsWhenTwoObjectsShareAName) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	g_lastErrorText.clear();
	g_errorCount = 0;
	model->getTracer()->addTraceErrorHandler(&CaptureTraceError);

	Resource* first = new Resource(model, "SharedName");
	Resource* second = new Resource(model, "SharedName");
	ASSERT_NE(first, nullptr);
	ASSERT_NE(second, nullptr);

	GenSerializer serializer(model);
	auto firstFields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	auto secondFields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	ModelDataDefinition::SaveInstance(firstFields.get(), first);
	ModelDataDefinition::SaveInstance(secondFields.get(), second);

	ASSERT_TRUE(serializer.put("SharedName", Util::TypeOf<Resource>(), first->getId(), firstFields.get()));
	EXPECT_EQ(g_errorCount, 0);

	ASSERT_TRUE(serializer.put("SharedName", Util::TypeOf<Resource>(), second->getId(), secondFields.get()));
	EXPECT_EQ(g_errorCount, 1);
	EXPECT_NE(g_lastErrorText.find("SharedName"), std::string::npos);

	// The collision is real: only the later entry survives under that key.
	auto readBack = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	ASSERT_TRUE(serializer.get("SharedName", readBack.get()));
	EXPECT_EQ(readBack->loadField("id", static_cast<unsigned int>(0)), second->getId());
}

TEST(SaveNameCollisionTest, XmlSerializerWarnsWhenTwoObjectsShareAName) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	g_lastErrorText.clear();
	g_errorCount = 0;
	model->getTracer()->addTraceErrorHandler(&CaptureTraceError);

	Resource* first = new Resource(model, "SharedName");
	Resource* second = new Resource(model, "SharedName");

	XmlSerializer serializer(model);
	auto firstFields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	auto secondFields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	ModelDataDefinition::SaveInstance(firstFields.get(), first);
	ModelDataDefinition::SaveInstance(secondFields.get(), second);

	ASSERT_TRUE(serializer.put("SharedName", Util::TypeOf<Resource>(), first->getId(), firstFields.get()));
	EXPECT_EQ(g_errorCount, 0);

	ASSERT_TRUE(serializer.put("SharedName", Util::TypeOf<Resource>(), second->getId(), secondFields.get()));
	EXPECT_EQ(g_errorCount, 1);
	EXPECT_NE(g_lastErrorText.find("SharedName"), std::string::npos);
}
