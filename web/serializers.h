#ifndef SERIALIZERS_H
#define SERIALIZERS_H

#include "crow/json.h"
#include "pedal.h"

crow::json::wvalue SerializePedalKnob(const PedalKnob& pedal_knob) {
  crow::json::wvalue serialized;
  serialized["name"] = pedal_knob.name;
  serialized["value"] = pedal_knob.value;
  serialized["tweak_amount"] = pedal_knob.tweak_amount;
  return serialized;
}

crow::json::wvalue SerializePedalInfo(const PedalInfo& pedal_info) {
  crow::json::wvalue serialized;
  serialized["name"] = pedal_info.name;
  serialized["state"] = pedal_info.state;

  std::vector<crow::json::wvalue> knobs;
  for (const auto& knob : pedal_info.knobs) {
    knobs.push_back(SerializePedalKnob(knob));
  }
  serialized["knobs"] = std::move(knobs);

  return serialized;
}

#endif /* SERIALIZERS_H */
