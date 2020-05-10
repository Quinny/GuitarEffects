#include "crow.h"

#include "web/handlers.h"

#include <fstream>
#include <string>
#include <vector>
#include "audio_transformer.h"
#include "pedal_registry.h"
#include "pedals/blues_drive_pedal.h"
#include "pedals/compressor_pedal.h"
#include "pedals/delay_pedal.h"
#include "pedals/echo_pedal.h"
#include "pedals/fuzz_pedal.h"
#include "playback.h"
#include "web/pedal_board.h"
#include "web/serializers.h"

int main() {
  crow::SimpleApp app;
  PedalBoard pedal_board;

  CROW_ROUTE(app, "/add_pedal/<string>")
  ([&pedal_board](const std::string& pedal_name) {
    auto pedal_factory =
        PedalRegistry::GetInstance().GetPedalFactoryOrNull(pedal_name);
    if (!pedal_factory) {
      return crow::response(404);
    }
    pedal_board.AddPedal((*pedal_factory)());
    return crow::response(200);
  });

  CROW_ROUTE(app, "/remove_pedal/<int>")
  ([&pedal_board](const int pedal_index) {
    pedal_board.RemovePedal(pedal_index);
    return crow::response(200);
  });

  CROW_ROUTE(app, "/active_pedals")
  ([&pedal_board]() {
    std::vector<crow::json::wvalue> active_pedals;
    auto pedals = pedal_board.GetPedals();
    for (const auto& pedal : pedals) {
      active_pedals.push_back(SerializePedalInfo(pedal));
    }

    crow::json::wvalue ret;
    ret = std::move(active_pedals);
    return crow::response(ret);
  });

  CROW_ROUTE(app, "/adjust_knob/<int>")
  ([&pedal_board](const crow::request& request, int pedal_index) {
    PedalKnob knob;
    knob.name = request.url_params.get("name");
    knob.value = std::stod(request.url_params.get("value"));
    pedal_board.AdjustKnob(pedal_index, knob);
    return crow::response(200);
  });

  CROW_ROUTE(app, "/available_pedals")
  ([&pedal_board]() {
    const auto registered_pedals =
        PedalRegistry::GetInstance().GetRegisteredPedals();
    std::vector<std::string> pedal_names;
    for (const auto& pedal_info : registered_pedals) {
      pedal_names.push_back(pedal_info.name);
    }

    crow::json::wvalue response;
    response = pedal_names;
    return crow::response(response);
  });

  CROW_ROUTE(app, "/<string>")
  (serve_static_files("static"));

  Playback pb(/* filename= */ "../recording");
  AudioTransformer at(
      [&pedal_board, &pb](SignalType input) {
        return pedal_board.Transform(pb.next());
      },
      3, 1, false);
  at.Start();

  app.port(8080).run();
}
