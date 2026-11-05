// Regression test for the GUI model save/load bug investigation:
// PersistenceDefaultImpl2::save() must write through a temp file and
// atomically replace the destination, rather than truncating the target
// file directly (which would corrupt/lose the previous good save if the
// process is interrupted mid-write). See PersistenceDefaultImpl2.cpp.

#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <string>
#include <unistd.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Decisions/PickUp.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/ExternalIntegration/PythonForG.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"
#include "plugins/data/ExternalIntegration/RSimulatorRunner.h"

TEST(PersistenceIntegrityTest, MissingConnectionTargetMakesLoadFail) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();

	const std::string filename = "/tmp/genesys_persistence_missing_target_"
		+ std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());

	std::ofstream file(filename);
	ASSERT_TRUE(file.is_open());
	file << "# Genesys Simulation Model\n"
		 << "0 Simulator \"GenESyS\" versionNumber=260705\n"
		 << "0 ModelInfo \"Broken connection\" version=\"1.0\"\n"
		 << "0 ModelSimulation \"\"\n"
		 << "140 Delay \"Broken\" delayExpression=\"1\" nexts=1 nextId=999999\n";
	file.close();

	EXPECT_EQ(simulator.getModelManager()->loadModel(filename), nullptr);
	::unlink(filename.c_str());
}

TEST(PersistenceIntegrityTest, InvalidLoadPreservesExistingModel) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	Resource* resource = new Resource(model, "MustSurviveFailedLoad");
	ASSERT_NE(resource, nullptr);

	const std::string filename = "/tmp/genesys_persistence_invalid_"
		+ std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());
	std::ofstream file(filename);
	ASSERT_TRUE(file.is_open());
	file << "# Genesys Simulation Model\n"
		 << "140 UnknownPersistenceType \"Broken\"\n";
	file.close();

	EXPECT_FALSE(model->load(filename));
	ASSERT_NE(model->getDataManager(), nullptr);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), "MustSurviveFailedLoad"), resource);
	::unlink(filename.c_str());
}

TEST(PersistenceAtomicSaveTest, SuccessfulSaveLeavesNoTemporaryFileBehind) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	new Resource(model, "AtomicSaveResource");

	const std::string filename = "/tmp/genesys_atomic_save_test_" + std::to_string(::getpid()) + ".gen";
	const std::string tempFilename = filename + ".tmp";
	::unlink(filename.c_str());
	::unlink(tempFilename.c_str());

	ASSERT_TRUE(model->save(filename));

	std::ifstream savedFile(filename);
	EXPECT_TRUE(savedFile.good());
	savedFile.close();

	std::ifstream tempFile(tempFilename);
	EXPECT_FALSE(tempFile.good()) << "temporary file should have been renamed away, not left behind";

	::unlink(filename.c_str());
	::unlink(tempFilename.c_str());
}

TEST(PersistenceAtomicSaveTest, SaveWithoutCheckPreservesIndependentDataDefinition) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	auto* resource = new Resource(model, "IndependentResource");
	auto* incomplete = new Decide(model, "IncompleteDecide");
	incomplete->addConditions("unfinished +");

	const std::string filename = "/tmp/genesys_save_without_check_"
		+ std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());

	ASSERT_TRUE(model->save(filename));
	ASSERT_TRUE(model->save(filename));
	std::ifstream savedFile(filename);
	ASSERT_TRUE(savedFile.good());
	const std::string contents((std::istreambuf_iterator<char>(savedFile)),
		std::istreambuf_iterator<char>());
	EXPECT_NE(contents.find("IndependentResource"), std::string::npos);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(),
		"IndependentResource"), resource);
	Model* loaded = simulator.getModelManager()->loadModel(filename);
	ASSERT_NE(loaded, nullptr);
	auto* loadedIncomplete = dynamic_cast<Decide*>(loaded->getComponentManager()->find("IncompleteDecide"));
	ASSERT_NE(loadedIncomplete, nullptr);
	ASSERT_EQ(loadedIncomplete->getConditions()->size(), 1u);
	EXPECT_EQ(loadedIncomplete->getConditions()->getAtRank(0), "unfinished +");

	::unlink(filename.c_str());
}

TEST(DecideConditionsEditorTest, SupportsOrderedDuplicateConditionsAndPositionalRemoval) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	auto* decide = new Decide(model, "OrderedDecide");
	SimulationControl* conditions = nullptr;
	for (SimulationControl* control : *decide->getSimulationControls()->list()) {
		if (control->getName() == "Conditions") conditions = control;
	}
	ASSERT_NE(conditions, nullptr);
	ASSERT_TRUE(conditions->supportsPositionalListEditing());
	conditions->insertListValueAt(0, "x > 0");
	conditions->insertListValueAt(1, "x > 0");
	conditions->insertListValueAt(1, "x == 0");
	ASSERT_EQ(decide->getConditions()->size(), 3u);
	EXPECT_EQ(decide->getConditions()->getAtRank(0), "x > 0");
	EXPECT_EQ(decide->getConditions()->getAtRank(1), "x == 0");
	auto* keepTarget = new Dispose(model, "KeepTarget");
	auto* removeTarget = new Dispose(model, "RemoveTarget");
	auto* keepConnection = new Connection{keepTarget, {0, ""}};
	auto* removeConnection = new Connection{removeTarget, {0, ""}};
	decide->getConnectionManager()->insertAtPort(0, keepConnection);
	decide->getConnectionManager()->insertAtPort(2, removeConnection);
	conditions->insertListValueAt(1, "inserted");
	EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), keepConnection);
	EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(3), removeConnection);
	conditions->removeListValueAt(3);
	EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), keepConnection);
	EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(3), nullptr);
	conditions->removeListValueAt(2);
	ASSERT_EQ(decide->getConditions()->size(), 2u);
	EXPECT_EQ(decide->getConditions()->getAtRank(1), "inserted");
	std::unique_ptr<PluginInformation> info(Decide::GetPluginInformation());
	EXPECT_EQ(info->getMinimumOutputs(), 1);
}

TEST(DecideConditionsEditorTest, OutputStatisticsTrackTheCurrentConditionCount) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	auto* decide = new Decide(model, "StatisticsDecide");
	decide->setReportStatistics(true);
	ModelDataDefinition::CreateInternalData(decide);
	ASSERT_EQ(decide->getInternalData()->size(), 1u);
	decide->insertConditionAt(0, "first");
	ASSERT_EQ(decide->getInternalData()->size(), 2u);
	decide->insertConditionAt(1, "second");
	ASSERT_EQ(decide->getInternalData()->size(), 3u);
	decide->removeConditionAt(0);
	ASSERT_EQ(decide->getInternalData()->size(), 2u);
	decide->removeConditionAt(0);
	EXPECT_EQ(decide->getInternalData()->size(), 1u);
}

TEST(PluginIdentityAndCodePersistenceTest, PickUpHasDistinctIdentityAndCppForGRoundTripsText) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	ASSERT_NE(simulator.getPluginManager()->find(Util::TypeOf<PickUp>()), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	auto* removeA = new Remove(model, "RemoveA");
	auto* removeB = new Remove(model, "RemoveB");
	auto* pickUp = new PickUp(model, "PickUpA");
	auto* python = new PythonForG(model, "PythonText");
	python->setInitBetweenReplicationCode("# λ\nstate = {\"quote\": \"value\"}");
	python->setOnDispatchEventCode("# arrival\nentity.setAttribute(\"label\", \"α\")");
	EXPECT_EQ(removeA->getClassname(), Util::TypeOf<Remove>());
	EXPECT_EQ(removeB->getClassname(), Util::TypeOf<Remove>());
	EXPECT_EQ(pickUp->getClassname(), Util::TypeOf<PickUp>());
	auto* cpp = new CppForG(model, "CppText");
	const std::string dispatch = R"(// λ
const char* s = "quoted"; \\path
)";
	cpp->setOnDispatchEventCode(dispatch);
	cpp->setInitBetweenReplicationCode("if (x) {\n  x++;\n}");
	cpp->setIncludesCode("#include <vector>\n// include text");
	const std::string filename = "/tmp/genesys_cppforg_roundtrip_" + std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());
	ASSERT_TRUE(model->save(filename));
	Model* loaded = simulator.getModelManager()->loadModel(filename);
	ASSERT_NE(loaded, nullptr);
	CppForG* loadedCpp = nullptr;
	PickUp* loadedPickUp = nullptr;
	PythonForG* loadedPython = nullptr;
	unsigned int loadedRemoves = 0;
	for (ModelComponent* component : *loaded->getComponentManager()) {
		if (component->getName() == "CppText") loadedCpp = dynamic_cast<CppForG*>(component);
		if (component->getClassname() == Util::TypeOf<PickUp>()) loadedPickUp = dynamic_cast<PickUp*>(component);
		if (component->getName() == "PythonText") loadedPython = dynamic_cast<PythonForG*>(component);
		if (component->getClassname() == Util::TypeOf<Remove>()) ++loadedRemoves;
	}
	ASSERT_NE(loadedCpp, nullptr);
	ASSERT_NE(loadedPickUp, nullptr);
	ASSERT_NE(loadedPython, nullptr);
	EXPECT_EQ(loadedRemoves, 2u);
	EXPECT_EQ(loadedCpp->getOnDispatchEventCode(), dispatch);
	EXPECT_EQ(loadedCpp->getInitBetweenReplicationCode(), "if (x) {\n  x++;\n}");
	EXPECT_EQ(loadedCpp->getIncludesCode(), "#include <vector>\n// include text");
	EXPECT_EQ(loadedPython->getInitBetweenReplicationCode(), "# λ\nstate = {\"quote\": \"value\"}");
	EXPECT_EQ(loadedPython->getOnDispatchEventCode(), "# arrival\nentity.setAttribute(\"label\", \"α\")");
	::unlink(filename.c_str());
}

TEST(SourceCodePropertyTest, PythonAndRCodeFieldsUseTheExistingCodeEditorContract) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	auto* python = new PythonForG(model, "PythonSource");
	auto* rSimulator = new RSimulator(model, "RSource");
	auto* runner = new RSimulatorRunner(model, "RRunnerSource");
	auto findControl = [](ModelDataDefinition* object, const std::string& property) {
		for (SimulationControl* control : *object->getSimulationControls()->list()) {
			if (control->getName() == property) return control;
		}
		return static_cast<SimulationControl*>(nullptr);
	};
	for (const auto& [object, property] : std::vector<std::pair<ModelDataDefinition*, std::string>>{
			{python, "OnDispatchEventCode"}, {rSimulator, "PreludeScript"}, {rSimulator, "CommandList"},
			{runner, "PreludeScript"}, {runner, "Command"}}) {
		SimulationControl* control = findControl(object, property);
		ASSERT_NE(control, nullptr) << property;
		EXPECT_EQ(control->preferredEditorHint(), SimulationControlEditorHint::CodeEditor) << property;
		control->setValue("# λ\nprint(\"text\")");
		EXPECT_EQ(control->getValue(), "# λ\nprint(\"text\")");
	}
}

TEST(SourceCodePropertyTest, RSimulatorAndRunnerPreserveScriptsAcrossGenRoundTrip) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	auto* component = new RSimulator(model, "RCommands");
	component->setPreludeScript("# λ prelude\noptions(stringsAsFactors = FALSE)");
	component->setCommandList("x <- \"quoted\"\nprint(x)");
	auto* runner = new RSimulatorRunner(model, "RRunner");
	runner->setPreludeScript("# λ runner\nset.seed(1)");
	runner->setCommand("message(\"done\")");
	const std::string filename = "/tmp/genesys_r_source_roundtrip_" + std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());
	ASSERT_TRUE(model->save(filename));
	EXPECT_NE(model->getDataManager()->getDataDefinition(Util::TypeOf<RSimulatorRunner>(),
		"RCommands.RSimulatorRunner"), nullptr);
	Model* loaded = simulator.getModelManager()->loadModel(filename);
	ASSERT_NE(loaded, nullptr);
	auto* loadedComponent = dynamic_cast<RSimulator*>(loaded->getComponentManager()->find("RCommands"));
	ASSERT_NE(loadedComponent, nullptr);
	EXPECT_EQ(loadedComponent->getPreludeScript(), "# λ prelude\noptions(stringsAsFactors = FALSE)");
	EXPECT_EQ(loadedComponent->getCommandList(), "x <- \"quoted\"\nprint(x)");
	auto* loadedRunner = dynamic_cast<RSimulatorRunner*>(loaded->getDataManager()->getDataDefinition(
		Util::TypeOf<RSimulatorRunner>(), "RRunner"));
	ASSERT_NE(loadedRunner, nullptr);
	EXPECT_EQ(loadedRunner->getPreludeScript(), "# λ runner\nset.seed(1)");
	EXPECT_EQ(loadedRunner->getCommand(), "message(\"done\")");
	::unlink(filename.c_str());
}

TEST(PersistenceAtomicSaveTest, SaveFailsCleanlyAndPreservesExistingFileWhenDestinationDirIsMissing) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	new Resource(model, "AtomicSaveResource");

	// A destination inside a nonexistent directory forces the temp-write step to fail
	// (std::ofstream cannot open a file whose parent directory doesn't exist).
	const std::string filename = "/tmp/genesys_atomic_save_missing_dir_"
		+ std::to_string(::getpid()) + "/model.gen";

	EXPECT_FALSE(model->save(filename));
}

TEST(PersistenceAtomicSaveTest, FailedTemporaryWritePreservesPreviousFileContents) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	new Resource(model, "PreservePriorSave");
	const std::string filename = "/tmp/genesys_atomic_save_preserve_" + std::to_string(::getpid()) + ".gen";
	const std::string tempFilename = filename + ".tmp";
	::unlink(filename.c_str());
	std::filesystem::remove_all(tempFilename);
	{
		std::ofstream previous(filename);
		ASSERT_TRUE(previous.is_open());
		previous << "previous valid model";
	}
	ASSERT_TRUE(std::filesystem::create_directory(tempFilename));
	EXPECT_FALSE(model->save(filename));
	std::ifstream previous(filename);
	ASSERT_TRUE(previous.good());
	const std::string contents((std::istreambuf_iterator<char>(previous)),
		std::istreambuf_iterator<char>());
	EXPECT_EQ(contents, "previous valid model");
	previous.close();
	std::filesystem::remove(tempFilename);
	::unlink(filename.c_str());
}
