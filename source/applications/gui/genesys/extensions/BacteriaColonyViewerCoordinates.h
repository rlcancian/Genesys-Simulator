#pragma once

#include <cstddef>

namespace BacteriaColonyViewerCoordinates {

// Returns the cell-space offset from the left/top grid edge for a centered
// Gro coordinate. Cell centers range from 0.5 to gridSize - 0.5.
inline double centeredCoordinateToCellOffset(double coordinate, std::size_t gridSize) {
	return coordinate + 0.5 * static_cast<double>(gridSize);
}

} // namespace BacteriaColonyViewerCoordinates
