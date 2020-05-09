#include "crow.h"

#include "web/handlers.h"

#include "audio_transformer.h"
#include "pedal_registry.h"
#include "pedals/delay_pedal.h"
#include "playback.h"
#include "web/pedal_board.h"
#include "web/serializers.h"

#include <fstream>
#include <string>
#include <vector>

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
