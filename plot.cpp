#include <cmath>
#include "matplotlib-cpp/matplotlibcpp.h"
#include "pedals/all_pedals.h"
#include "pedals/fuzz_pedal.h"
#include "playback.h"

// Apply the pedal to each of the frames and add an offset to split up the
// graph.
std::vector<SignalType> ApplyPedal(const std::vector<SignalType>& frames,
                                   Pedal& pedal, int offset) {
  std::vector<SignalType> output;
  for (const auto& frame : frames) {
    output.push_back(pedal.Transform(frame) + offset);
  }
  return output;
}

int main() {
  // Load the recording and pre-seed the x-axis with each frame number.
  Playback pb("recording");
  std::vector<SignalType> xs;
  for (int i = 0; i < pb.frames().size(); ++i) {
    xs.push_back(i);
  }

  // Add a bunch of pedals to compare the signals between.
  std::vector<std::unique_ptr<Pedal>> pedals;
  pedals.push_back(std::make_unique<FuzzPedal>());
  pedals.push_back(std::make_unique<AutoWahPedal>());
  pedals.push_back(std::make_unique<DistortionPedal>());
  pedals.push_back(std::make_unique<SigmoidPedal>());

  // Plot the clean signal.
  matplotlibcpp::named_plot("Clean", xs, pb.frames());
  std::vector<SignalType> origin_v(pb.frames().size(), 0);
  matplotlibcpp::plot(xs, origin_v, "--k");

  // Plot each pedal signal and offset by 3 to prevent the graphs from
  // overlapping.
  int offset = 3;
  for (auto& pedal : pedals) {
    const auto pedal_name = pedal->Describe().name;
    matplotlibcpp::named_plot(pedal_name, xs,
                              ApplyPedal(pb.frames(), *pedal, offset));

    std::vector<SignalType> limit_v(pb.frames().size(), offset - 1.5);
    matplotlibcpp::plot(xs, limit_v, "--k");

    std::vector<SignalType> origin_v(pb.frames().size(), offset);
    matplotlibcpp::plot(xs, origin_v, "--k");
    offset += 3;
  }

  matplotlibcpp::title("Pedal Signal Comparison");
  // The default location of the legend ("best") is super expensive to compute
  // with large plots.
  matplotlibcpp::legend({{"loc", "upper right"}});
  matplotlibcpp::show();
}
