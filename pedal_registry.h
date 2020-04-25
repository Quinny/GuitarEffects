#ifndef PEDAL_REGISTRY_H
#define PEDAL_REGISTRY_H

#include "pedal.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// A registry which holds factory functions for creating different types of
// pedals. Pedals should self-register themselves with the register using the
// REGISTER_PEDAL macro. This class is a singleton.
class PedalRegistry {
 public:
  using PedalFactoryFn = std::function<std::unique_ptr<Pedal>()>;

  // Get the singleton PedalRegistry instance.
  static PedalRegistry& GetInstance() {
    static auto* instance = new PedalRegistry();
    return *instance;
  }

  // Add a new pedal to the registry. This should only be accessed via the
  // REGISTER_PEDAL macro.
  //
  // Returns a bool because we need it for the macro to work properly. The C++
  // compiler needs the registration line to be an assignment otherwise the code
  // won't compile because it parses are a declaration.
  bool RegisterPedal(const std::string& pedal_name, PedalFactoryFn factory_fn) {
    pedal_factories_.emplace(pedal_name, std::move(factory_fn));
    return true;
  }

  // Returns the factory associated with the provided pedal or null if it
  // doesn't exist.
  const PedalFactoryFn* GetPedalFactoryOrNull(const std::string& pedal_name) {
    auto factory_it = pedal_factories_.find(pedal_name);
    return factory_it == pedal_factories_.end() ? nullptr : &factory_it->second;
  }

 private:
  PedalRegistry() = default;
  std::unordered_map<std::string, PedalFactoryFn> pedal_factories_;
};

// Generates a random variable name. This is used to prevent collisions between
// different pedals registering themselves.
#define RANDOM_NAME(prefix) JOIN(prefix, __COUNTER__)
#define JOIN(symbol1, symbol2) _DO_JOIN(symbol1, symbol2)
#define _DO_JOIN(symbol1, symbol2) symbol1##symbol2

// Register a pedal with the registry.
#define REGISTER_PEDAL(...)                          \
  auto RANDOM_NAME(qp_pedal_registration_reserved) = \
      PedalRegistry::GetInstance().RegisterPedal(__VA_ARGS__);

#endif /* PEDAL_REGISTRY_H */
