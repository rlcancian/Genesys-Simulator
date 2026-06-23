// Integration of the user-defined local rule into the GenESyS component CellularAutomataComp:
// verifies semantic checking (_check) and persistence (_loadInstance/_saveInstance) of the whole
// cellular-automaton configuration, including a USERDEFINED rule compiled at runtime.

#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_UserDefined.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Persistence.h"

#include <string>

namespace {

// Minimal Persistence_if so a PersistenceRecord can store/replay fields in a unit test.
class FakePersistence : public Persistence_if {
public:
	bool save(std::string) override { return false; }
	bool load(std::string) override { return false; }
	bool hasChanged() override { return false; }
	void setHasChanged(bool) override {}
	bool getOption(Persistence_if::Options) override { return false; }
	void setOption(Persistence_if::Options, bool) override {}
	std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

// Exposes the protected lifecycle hooks for testing.
class ComponentProbe : public CellularAutomataComp {
public:
	ComponentProbe(Model* model, const std::string& name) : CellularAutomataComp(model, name) {}
	bool CheckProbe(std::string* errorMessage) { return _check(errorMessage); }
	void SaveProbe(PersistenceRecord* fields, bool saveDefaultValues = true) {
		_saveInstance(fields, saveDefaultValues);
	}
	bool LoadProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
};

void configureValidUserDefined(ComponentProbe& comp, const std::string& ruleSource) {
	comp.setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	comp.setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	comp.getlattice()->setDimensions({9}); // 1D lattice: required by the (universal) semantic check for a CENTERED neighborhood
	comp.setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	comp.setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	comp.setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	comp.setUserDefinedRuleSource(ruleSource);
	comp.setLocalRuleType(CellularAutomataComp::LocalRuleType::USERDEFINED);
}

const std::string kRule90 = LocalRule_UserDefined::wrapBody("return neighbors[0] ^ neighbors[1];");

} // namespace

TEST(CellularAutomataComponent, CheckPassesAndBuildsUserDefinedRule) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ComponentProbe comp(model, "CA_UserDefined");
	configureValidUserDefined(comp, kRule90);

	std::string error;
	ASSERT_TRUE(comp.CheckProbe(&error)) << error;

	LocalRule_UserDefined* rule = dynamic_cast<LocalRule_UserDefined*>(comp.getlocalRule());
	ASSERT_NE(rule, nullptr) << "USERDEFINED check should install a LocalRule_UserDefined";
	EXPECT_TRUE(rule->isReady()) << "the user rule should be compiled and loaded after check";
}

TEST(CellularAutomataComponent, CheckRejectsUserDefinedWithoutSource) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ComponentProbe comp(model, "CA_NoSource");
	configureValidUserDefined(comp, ""); // empty source

	std::string error;
	EXPECT_FALSE(comp.CheckProbe(&error));
	EXPECT_NE(error.find("requires source code"), std::string::npos) << error;
}

TEST(CellularAutomataComponent, CheckRejectsUserDefinedWithBadSource) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ComponentProbe comp(model, "CA_BadSource");
	configureValidUserDefined(comp, "this is not valid c++ <<<");

	std::string error;
	EXPECT_FALSE(comp.CheckProbe(&error));
	EXPECT_NE(error.find("failed to compile"), std::string::npos) << error;
}

TEST(CellularAutomataComponent, CheckRejectsMissingCellularAutomataType) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ComponentProbe comp(model, "CA_Empty");
	// Nothing configured: the cellular automata is null.
	std::string error;
	EXPECT_FALSE(comp.CheckProbe(&error));
	EXPECT_FALSE(error.empty());
}

TEST(CellularAutomataComponent, PersistenceRoundTripPreservesConfiguration) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	ComponentProbe saved(model, "CA_Saved");
	configureValidUserDefined(saved, kRule90);
	// Non-default values for the universal update-policy fields, to prove they also round-trip.
	saved.setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::BLOCKS);
	saved.setUpdateBlockSize(3);
	saved.setRandomSeed(42);

	FakePersistence persistence;
	PersistenceRecord fields(persistence);
	saved.SaveProbe(&fields, true);

	ComponentProbe loaded(model, "CA_Loaded");
	ASSERT_TRUE(loaded.LoadProbe(&fields));

	EXPECT_EQ(loaded.getCellularAutomataType(), CellularAutomataComp::CellularAutomataType::CLASSIC);
	EXPECT_EQ(loaded.getLatticeType(), CellularAutomataComp::LatticeType::RETICULAR);
	EXPECT_EQ(loaded.getNeighboorhoodType(), CellularAutomataComp::NeighboorhoodType::CENTERED);
	EXPECT_EQ(loaded.geBoundaryType(), CellularAutomataComp::BoundaryType::FIXED);
	EXPECT_EQ(loaded.getStateSetType(), CellularAutomataComp::StateSetType::ENUMERATED);
	EXPECT_EQ(loaded.getlocalRuleType(), CellularAutomataComp::LocalRuleType::USERDEFINED);
	EXPECT_EQ(loaded.getUserDefinedRuleSource(), kRule90);
	ASSERT_NE(loaded.getlattice(), nullptr);
	EXPECT_EQ(loaded.getlattice()->getDimensions(), (std::vector<unsigned short>{9})); // lattice dimensions round-trip
	EXPECT_EQ(loaded.getUpdatePolicyType(), CellularAutomataComp::UpdatePolicyType::BLOCKS);
	EXPECT_EQ(loaded.getUpdateBlockSize(), 3u);
	EXPECT_EQ(loaded.getRandomSeed(), 42u);

	// The reloaded configuration must still pass the semantic check (rebuilds and recompiles the rule).
	std::string error;
	EXPECT_TRUE(loaded.CheckProbe(&error)) << error;
}
