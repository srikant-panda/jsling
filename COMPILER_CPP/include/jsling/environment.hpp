#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "value.hpp"

namespace jsling {

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr);

    void define(const std::string& name, JSValue value);
    JSValue get(const std::string& name) const;
    void set(const std::string& name, JSValue value);
    bool has(const std::string& name) const;

private:
    std::shared_ptr<Environment> parent_;
    std::unordered_map<std::string, JSValue> vars_;
};

} // namespace jsling
