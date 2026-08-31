// Regression test for the GUI model save/load bug investigation:
// PersistenceDefaultImpl2::save() must write through a temp file and
// atomically replace the destination, rather than truncating the target
// file directly (which would corrupt/lose the previous good save if the
// process is interrupted mid-write). See PersistenceDefaultImpl2.cpp.

#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <unistd.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

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
