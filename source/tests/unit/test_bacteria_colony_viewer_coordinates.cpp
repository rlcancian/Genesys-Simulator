#include "../../applications/gui/genesys/extensions/BacteriaColonyViewerCoordinates.h"

#include <gtest/gtest.h>

TEST(BacteriaColonyViewerCoordinates, CenteredEightByEightDomainMapsSeedsInsideTheGrid) {
	using BacteriaColonyViewerCoordinates::centeredCoordinateToCellOffset;

	EXPECT_DOUBLE_EQ(centeredCoordinateToCellOffset(-3.5, 8), 0.5);
	EXPECT_DOUBLE_EQ(centeredCoordinateToCellOffset(0.0, 8), 4.0);
	EXPECT_DOUBLE_EQ(centeredCoordinateToCellOffset(3.5, 8), 7.5);
}

TEST(BacteriaColonyViewerCoordinates, OddGridOriginMapsToItsCenterCell) {
	EXPECT_DOUBLE_EQ(BacteriaColonyViewerCoordinates::centeredCoordinateToCellOffset(0.0, 3), 1.5);
}
