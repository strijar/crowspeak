#pragma once

// Compile-time hardware selection. Both adapters implement the raw-radio
// contract consumed by LoRaInterface; framing and queue ownership stay there.
#include "config/BoardConfig.h"
#include "radio/SX1262.h"
using BoardRadio = SX1262;
