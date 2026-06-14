#pragma once
#include "value.hpp"
#include <string>

namespace jsling {

std::string toDisplayString(const JSValue& value);
std::string inspect(const JSValue& value, int depth = 0);
std::string formatNumber(double n);

} // namespace jsling
