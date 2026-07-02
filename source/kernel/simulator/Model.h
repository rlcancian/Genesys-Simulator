#pragma once

// Compatibility shim for DCS T6.2 controlled evaluation.
// The original PR targeted currentStable and still includes kernel/simulator/Model.h.
// In 2026-1 the header lives under kernel/simulator/model/Model.h.
#include "model/Model.h"
