#include <fstream>
#include <string>
#include <vector>

#include "visualization/visualizer.h"

#include "audio_transformer.h"
#include "crow.h"
#include "playback.h"
#include "web/handlers.h"
#include "web/pedal_board.h"

int main(int argc, char* argv[]) {
  bool in_debug_mode = argc > 1 && strcmp(argv[1], "debug") == 0;

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

  CROW_ROUTE(app, "/")
  ([]() {
    StaticFileHandler static_file_handler(/* directory = */ "static");
    return static_file_handler("index.html");
  });

  StaticFileHandler static_file_handler(/* directory = */ "static");
  CROW_ROUTE(app, "/<string>")(static_file_handler);

  FrameBuffer frame_buffer(/* max_size= */ 44100);
  Visualizer v(&frame_buffer, /* fps= */ 30);

  Playback pb(/* filename= */ "../recording");
  auto transform = [in_debug_mode, &pedal_board, &pb,
                    &frame_buffer](SignalType input) {
    auto out = pedal_board.Transform(in_debug_mode ? pb.next() : input);
    frame_buffer.Add(out);
    return out;
  };

  AudioTransformer::DumpDeviceInfo();
  AudioTransformer at(transform,
                      /* input_device_index= */ in_debug_mode ? 0 : 3,
                      /* output_device_index= */ in_debug_mode ? 1 : 3,
                      /* treat_input_as_mono= */ !in_debug_mode);
  at.Start();

  if (!in_debug_mode) {
    app.loglevel(crow::LogLevel::WARNING);
  }

  std::thread app_thread([&]() { app.port(in_debug_mode ? 8080 : 80).run(); });
  v.BlockingStart();
  app_thread.join();
}
