#pragma once

#include <vector>

#include "qream/ir.h"

std::vector<uint8_t> transpile_to_arm64(const std::vector<Operation> &ops);
