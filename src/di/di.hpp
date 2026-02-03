#pragma once

#include "env/env_manager.hpp"    // NOLINT:
#include <functional>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace bot {

#define GET(di, type) di.Get<type>()

#define GET_ENV(di, name) di.Get<EnvManager>()->Get(name)

#define REGISTER(di, type, ...) \
    di.Register<type>(          \
        [&](DiContainer& di) { return std::make_shared<type>(__VA_ARGS__); })

#define REGISTER_I(di, interface, type, ...) \
    di.Register<interface>(                  \
        [&](DiContainer& di) { return std::make_shared<type>(__VA_ARGS__); })

class DiContainer {
private:
    std::map<std::string, std::shared_ptr<void>> instances_;
    std::map<std::string, std::function<std::shared_ptr<void>(DiContainer&)>> factories_;

public:
    template <typename T> std::shared_ptr<T> Get(const std::string& name) {
        if (instances_.find(name) != instances_.end()) {

            return std::static_pointer_cast<T>(instances_.at(name));
        }

        if (factories_.find(name) == factories_.end()) {
            throw std::runtime_error("Factory method for (" + name + ") does not exists");
        }

        std::shared_ptr<void> instance = factories_[name](*this);

        instances_[name] = instance;

        return std::static_pointer_cast<T>(instance);
    }

    template <typename T> std::shared_ptr<T> Get() {
        std::string name = typeid(T).name();
        return Get<T>(name);
    }

    template <typename T>
    void Register(const std::string& name,
                  std::function<std::shared_ptr<void>(DiContainer&)> factory_method) {

        if (factories_.find(name) != factories_.end()) {
            throw std::runtime_error("Type with name (" + name + ") already exists");
        }
        factories_[name] = factory_method;
    }

    template <typename T>
    void Register(std::function<std::shared_ptr<void>(DiContainer&)> factory_method) {
        std::string name = typeid(T).name();
        Register<T>(name, factory_method);
    }
};

template <typename T> struct Scoped {

    explicit Scoped(const std::shared_ptr<T>& ptr) : ptr(ptr) {}

    std::shared_ptr<T> ptr;

    T* operator->() { return ptr.get(); }
    T& operator*() { return *ptr.get(); }
};

template <typename T> Scoped<T> Scope(DiContainer& c, std::string name) {
    return Scoped<T>{c.Get<T>(name)};
}

template <typename T> Scoped<T> Scope(DiContainer& c) { return Scoped<T>{c.Get<T>()}; }

}    // namespace bot
