#pragma once
#include <stdexcept>
#include <string>

namespace jsling {

class JSError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SyntaxError : public JSError { using JSError::JSError; };
class ReferenceError : public JSError { using JSError::JSError; };
class TypeError : public JSError { using JSError::JSError; };
class RangeError : public JSError { using JSError::JSError; };

// Control flow signals (ReturnSignal, BreakSignal, ContinueSignal) 
// are defined in value.hpp since ReturnSignal needs JSValue.

} // namespace jsling
