// moon.h — local moon-phase calculation. No network needed.
#pragma once

#include "model.h"

namespace moon {

// Compute moon information for the given UTC epoch time.
MoonInfo compute(time_t utc);

}  // namespace moon
