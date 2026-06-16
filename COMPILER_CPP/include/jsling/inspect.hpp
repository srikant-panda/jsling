#pragma once
#include "value.hpp"
#include <string>

namespace jsling {

std::string toDisplayString(const JSValue& value);
std::string inspect(const JSValue& value, int depth = 0);
std::string colorInspect(const JSValue& value, int depth = 0);
std::string colorDisplayString(const JSValue& value);
std::string formatNumber(double n);

// Spell-check: suggest a correction for an unknown identifier
std::string suggestIdentifier(const std::string& name, const std::vector<std::string>& candidates);

// Error formatting helpers
std::string formatError(const std::string& type, const std::string& message, int line,
                        const std::string& sourceLine = "", const std::string& suggestion = "");
std::string formatRuntimeError(const std::string& type, const std::string& message,
                               const std::string& source = "", int line = -1);

} // namespace jsling
