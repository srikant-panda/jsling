#pragma once
#include "value.hpp"
#include "environment.hpp"
#include <vector>
#include <string>

namespace jsling {

void setupBuiltins(std::shared_ptr<Environment> env);

} // namespace jsling
