#ifndef HANDLERS_H
#define HANDLERS_H

#include "crow.h"
#include "pedal_registry.h"
#include "pedals/all_pedals.h"
#include "web/pedal_board.h"
#include "web/serializers.h"

#include <fstream>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

class StaticFileHandler {
public:
  StaticFileHandler(const std::string& directory) : directory_(directory) {}

  crow::response operator()(const std::string& filename) const {
    // Protect against escaping the static file directory.
    if (filename.find("..") != std::string::npos) {
      return crow::response(400);
    }

    const auto file_path = directory_ + "/" + filename;
    std::ifstream file_stream(file_path);
    if (!file_stream) {
      return crow::response(404);
    }

    static constexpr int kReadFileBufferSize = 1024;
    char read_buffer[kReadFileBufferSize];
    std::string response_body;
    while (file_stream) {
      file_stream.read(read_buffer, kReadFileBufferSize);
      std::copy(read_buffer, read_buffer + file_stream.gcount(),
                std::back_inserter(response_body));
    }

    return crow::response(std::move(response_body));
  }

private:
  std::string directory_;
};

class AvailablePedalHandler {
public:
  crow::response operator()() const {
    const auto registered_pedals =
        PedalRegistry::GetInstance().GetRegisteredPedals();
    std::vector<std::string> pedal_names;
    for (const auto& pedal_info : registered_pedals) {
      pedal_names.push_back(pedal_info.name);
    }

    crow::json::wvalue response;
    response = pedal_names;
    return crow::response(response);
  }
};

class ActivePedalHandler {
public:
  ActivePedalHandler(const PedalBoard* pedal_board)
      : pedal_board_(pedal_board) {}

  crow::response operator()() const {
    std::vector<crow::json::wvalue> active_pedals;
    auto pedals = pedal_board_->GetPedals();
    for (const auto& pedal : pedals) {
      active_pedals.push_back(SerializePedalInfo(pedal));
    }

    crow::json::wvalue response;
    response["pedals"] = std::move(active_pedals);
    return response;
  }

private:
  const PedalBoard* pedal_board_;
};

class UpdatesHandler {
public:
  void RegisterConnection(crow::websocket::connection* connection) {
    std::lock_guard<std::mutex> lock(mu_);
    connections_.insert(connection);
  }

  void RemoveConnection(crow::websocket::connection* connection) {
    std::lock_guard<std::mutex> lock(mu_);
    connections_.erase(connection);
  }

  void OnUpdate() {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto* connection : connections_) {
      connection->send_text("ping");
    }
  }

private:
  std::mutex mu_;
  std::unordered_set<crow::websocket::connection*> connections_;
};

class RemovePedalHandler {
public:
  RemovePedalHandler(PedalBoard* pedal_board, UpdatesHandler* updates)
      : pedal_board_(pedal_board), updates_(updates) {}

  crow::response operator()(int pedal_index) const {
    pedal_board_->RemovePedal(pedal_index);
    updates_->OnUpdate();
    return crow::response(200);
  }

private:
  mutable PedalBoard* pedal_board_;
  mutable UpdatesHandler* updates_;
};

class AdjustKnobHandler {
public:
  AdjustKnobHandler(PedalBoard* pedal_board, UpdatesHandler* updates)
      : pedal_board_(pedal_board), updates_(updates) {}

  crow::response operator()(const crow::request& request,
                            int pedal_index) const {
    auto* name_param = request.url_params.get("name");
    auto* value_param = request.url_params.get("value");

    if (!name_param || !value_param) {
      return crow::response(400);
    }

    PedalKnob knob;
    knob.name = name_param;
    knob.value = std::stod(value_param);
    pedal_board_->AdjustKnob(pedal_index, knob);
    updates_->OnUpdate();
    return crow::response(200);
  }

private:
  mutable PedalBoard* pedal_board_;
  mutable UpdatesHandler* updates_;
};

class PushButtonHandler {
public:
  PushButtonHandler(PedalBoard* pedal_board, UpdatesHandler* updates)
      : pedal_board_(pedal_board), updates_(updates) {}

  crow::response operator()(int pedal_index) const {
    pedal_board_->Push(pedal_index);
    updates_->OnUpdate();
    return crow::response(200);
  }

private:
  mutable PedalBoard* pedal_board_;
  mutable UpdatesHandler* updates_;
};

class AddPedalHandler {
public:
  AddPedalHandler(PedalBoard* pedal_board, UpdatesHandler* updates)
      : pedal_board_(pedal_board), updates_(updates) {}

  crow::response operator()(const std::string& pedal_name) const {
    auto pedal_factory =
        PedalRegistry::GetInstance().GetPedalFactoryOrNull(pedal_name);
    if (!pedal_factory) {
      return crow::response(404);
    }

    pedal_board_->AddPedal((*pedal_factory)());
    updates_->OnUpdate();
    return crow::response(200);
  }

private:
  mutable PedalBoard* pedal_board_;
  mutable UpdatesHandler* updates_;
};

#endif /* HANDLERS_H */
