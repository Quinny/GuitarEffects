#ifndef HANDLERS_H
#define HANDLERS_H

#include "crow.h"

#include <fstream>
#include <string>

std::function<crow::response(const std::string& filename)> serve_static_files(
    const std::string& directory) {
  return [directory](const std::string& filename) {
    // TODO: Don't allow filenames with "." to prevent people from reading
    // random files.
    std::ifstream file_stream(directory + "/" + filename);

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

    // TODO: Do I need to set the mime type?
    return crow::response(std::move(response_body));
  };
}

#endif /* HANDLERS_H */
