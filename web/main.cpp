#include <fstream>
#include <string>
#include <vector>

#include "audio_transformer.h"
#include "crow.h"
#include "playback.h"
#include "web/handlers.h"
#include "web/pedal_board.h"

int main() {
  crow::SimpleApp app;
  PedalBoard pedal_board;

  AddPedalHandler add_pedal_handler(&pedal_board);
  CROW_ROUTE(app, "/add_pedal/<string>")(add_pedal_handler);

  RemovePedalHandler remove_pedal_handler(&pedal_board);
  CROW_ROUTE(app, "/remove_pedal/<int>")(remove_pedal_handler);

  ActivePedalHandler active_pedal_handler(&pedal_board);
  CROW_ROUTE(app, "/active_pedals")(active_pedal_handler);

  AdjustKnobHandler adjust_knob_handler(&pedal_board);
  CROW_ROUTE(app, "/adjust_knob/<int>")(adjust_knob_handler);

  AvailablePedalHandler available_pedal_handler;
  CROW_ROUTE(app, "/available_pedals")(available_pedal_handler);

  StaticFileHandler static_file_handler(/* directory = */ "static");
  CROW_ROUTE(app, "/<string>")(static_file_handler);

  Playback pb(/* filename= */ "../recording");
  AudioTransformer at(
      [&pedal_board, &pb](SignalType input) {
        return pedal_board.Transform(pb.next());
      },
      3, 1, false);
  at.Start();

  app.port(8080).run();
}
