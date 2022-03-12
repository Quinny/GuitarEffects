#define CROW_MAIN

#include <fstream>
#include <string>
#include <vector>

#include "visualization/visualizer.h"

#include "audio_transformer.h"
#include "crow.h"
#include "playback.h"
#include "rtaudio/RtAudio.h"
#include "web/handlers.h"
#include "web/pedal_board.h"

struct AudioDevice {
  unsigned int device_index;
  RtAudio::DeviceInfo info;
};

std::vector<AudioDevice> GetAllDevices() {
  std::vector<AudioDevice> devices;
  RtAudio audio_interface;
  unsigned int num_devices = audio_interface.getDeviceCount();
  for (unsigned int i = 0; i < num_devices; i++) {
    auto info = audio_interface.getDeviceInfo(i);
    if (!info.probed) {
      continue;
    }
    devices.push_back(AudioDevice{.device_index = i, .info = info});
  }
  return devices;
}

std::vector<AudioDevice>
FilterDevices(const std::vector<AudioDevice>& devices,
              const std::function<bool(const AudioDevice&)> predicate) {
  std::vector<AudioDevice> filtered_devices;
  for (const auto& device : devices) {
    if (predicate(device)) {
      filtered_devices.push_back(device);
    }
  }
  return filtered_devices;
}

void ShowDevices(const std::vector<AudioDevice>& devices) {
  for (const auto& device : devices) {
    std::cout << device.device_index << " - " << device.info.name << std::endl;
  }
}

template <typename T> struct MaybeError {
  std::string error_message;
  T value;
};

struct DeviceSelections {
  AudioDevice input_device;
  AudioDevice output_device;
};

void WriteSelections(const DeviceSelections& selections) {
  std::ofstream devices_file("devices.txt");
  devices_file << selections.input_device.info.name << "\n"
               << selections.output_device.info.name;
}

MaybeError<DeviceSelections>
ReadSelectionsFromFile(const std::vector<AudioDevice>& all_devices) {
  std::ifstream devices_file("devices.txt");
  if (!devices_file) {
    return MaybeError<DeviceSelections>{.error_message =
                                            "Devices file not found"};
  }

  std::string input_device_name;
  std::getline(devices_file, input_device_name);
  auto input_device =
      FilterDevices(all_devices, [&input_device_name](const auto& device) {
        return device.info.inputChannels > 0 &&
               device.info.name == input_device_name;
      });
  if (input_device.empty()) {
    return MaybeError<DeviceSelections>{
        .error_message = "Input device '" + input_device_name + "' not found"};
  }

  std::string output_device_name;
  std::getline(devices_file, output_device_name);
  auto output_device =
      FilterDevices(all_devices, [&output_device_name](const auto& device) {
        return device.info.outputChannels > 0 &&
               device.info.name == output_device_name;
      });
  if (output_device.empty()) {
    return MaybeError<DeviceSelections>{.error_message = "output device '" +
                                                         output_device_name +
                                                         "' not found"};
  }

  return MaybeError<DeviceSelections>{
      .value = DeviceSelections{.input_device = input_device.front(),
                                .output_device = output_device.front()}};
}

DeviceSelections SelectDevices() {
  auto all_devices = GetAllDevices();
  MaybeError<DeviceSelections> file_selections =
      ReadSelectionsFromFile(all_devices);
  if (file_selections.error_message.empty()) {
    return file_selections.value;
  }

  std::cout << file_selections.error_message << std::endl;
  std::cout << "Available input devices:" << std::endl;
  ShowDevices(FilterDevices(all_devices, [](const AudioDevice& device) {
    return device.info.inputChannels > 0;
  }));
  int selected_input_device;
  std::cout << "> ";
  std::cin >> selected_input_device;

  std::cout << "Available output devices:" << std::endl;
  ShowDevices(FilterDevices(all_devices, [](const AudioDevice& device) {
    return device.info.outputChannels > 0;
  }));
  int selected_output_device;
  std::cout << "> ";
  std::cin >> selected_output_device;

  RtAudio audio_interface;
  DeviceSelections selections{
      .input_device = all_devices[selected_input_device],
      .output_device = all_devices[selected_output_device]};
  WriteSelections(selections);
  return selections;
}

int main(int argc, char* argv[]) {
  bool in_debug_mode = argc > 1 && strcmp(argv[1], "debug") == 0;

  crow::SimpleApp app;
  PedalBoard pedal_board;

  ActivePedalHandler active_pedal_handler(&pedal_board);
  CROW_ROUTE(app, "/active_pedals")(active_pedal_handler);

  UpdatesHandler updates_handler;
  CROW_ROUTE(app, "/updates")
      .websocket()
      .onopen([&](crow::websocket::connection& conn) {
        updates_handler.RegisterConnection(&conn);
      })
      .onclose(
          [&](crow::websocket::connection& conn, const std::string& reason) {
            updates_handler.RemoveConnection(&conn);
          });

  AddPedalHandler add_pedal_handler(&pedal_board, &updates_handler);
  CROW_ROUTE(app, "/add_pedal/<string>")(add_pedal_handler);

  RemovePedalHandler remove_pedal_handler(&pedal_board, &updates_handler);
  CROW_ROUTE(app, "/remove_pedal/<int>")(remove_pedal_handler);

  PushButtonHandler push_button_handler(&pedal_board, &updates_handler);
  CROW_ROUTE(app, "/push_button/<int>")(push_button_handler);

  AdjustKnobHandler adjust_knob_handler(&pedal_board, &updates_handler);
  CROW_ROUTE(app, "/adjust_knob/<int>")(adjust_knob_handler);

  AvailablePedalHandler available_pedal_handler;
  CROW_ROUTE(app, "/available_pedals")(available_pedal_handler);

  CROW_ROUTE(app, "/")
  ([]() {
    StaticFileHandler static_file_handler(/* directory = */ "static");
    return static_file_handler("index.html");
  });

  TemperatureHandler temperature_handler;
  CROW_ROUTE(app, "/temp")(temperature_handler);

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

  auto selections = SelectDevices();
  AudioTransformer at(transform, selections.input_device.device_index,
                      selections.output_device.device_index);
  at.Start();

  if (!in_debug_mode) {
    app.loglevel(crow::LogLevel::WARNING);
  }

  std::thread app_thread([&]() { app.port(in_debug_mode ? 8080 : 80).run(); });
  v.BlockingStart();
  app_thread.join();
}
