#include "jsling/environment.hpp"
#include "jsling/errors.hpp"

namespace jsling {

Environment::Environment(std::shared_ptr<Environment> parent) 
    : isFunctionOrGlobal(parent == nullptr), parent_(std::move(parent)) {}

std::shared_ptr<Environment> Environment::getFunctionOrGlobalEnv() {
    if (isFunctionOrGlobal) {
        return shared_from_this();
    }
    if (parent_) {
        return parent_->getFunctionOrGlobalEnv();
    }
    return shared_from_this();
}

void Environment::define(const std::string& name, JSValue value) {
    vars_[name] = std::move(value);
}

JSValue Environment::get(const std::string& name) const {
    auto it = vars_.find(name);
    if (it != vars_.end()) return it->second;
    if (parent_) return parent_->get(name);
    throw ReferenceError(name + " is not defined");
}

void Environment::set(const std::string& name, JSValue value) {
    auto it = vars_.find(name);
    if (it != vars_.end()) {
        it->second = std::move(value);
        return;
    }
    if (parent_) {
        parent_->set(name, std::move(value));
        return;
    }
    throw ReferenceError(name + " is not defined");
}

bool Environment::has(const std::string& name) const {
    if (vars_.find(name) != vars_.end()) return true;
    if (parent_) return parent_->has(name);
    return false;
}

std::vector<std::string> Environment::visibleNames() const {
    std::vector<std::string> names;
    for (const auto& [k, v] : vars_) names.push_back(k);
    if (parent_) {
        auto parentNames = parent_->visibleNames();
        names.insert(names.end(), parentNames.begin(), parentNames.end());
    }
    return names;
}

} // namespace jsling
