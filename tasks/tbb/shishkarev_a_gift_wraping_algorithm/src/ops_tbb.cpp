#include "tbb/shishkarev_a_gift_wraping_algorithm/include/ops_tbb.hpp"

#include <tbb/tbb.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <set>
#include <vector>

#include "oneapi/tbb/task_arena.h"
#include "oneapi/tbb/task_group.h"

std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> shishkarev_a_gift_wraping_algorithm_tbb::RemoveDuplicates(
    const std::vector<Vertex>& points) {
  if (points.empty()) return {};

  std::vector<Vertex> unique_points = points;
  std::sort(unique_points.begin(), unique_points.end());
  auto last = std::unique(unique_points.begin(), unique_points.end());
  unique_points.erase(last, unique_points.end());
  return unique_points;
}

namespace {

void FindStartPoint(const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& input, int& start_idx,
                    shishkarev_a_gift_wraping_algorithm_tbb::Vertex& start_point) {
  start_point = {INT_MAX, INT_MAX};
  for (size_t i = 0; i < input.size(); i++) {
    if ((input[i].y < start_point.y) || ((input[i].y == start_point.y) && (input[i].x > start_point.x))) {
      start_idx = static_cast<int>(i);
      start_point = input[i];
    }
  }
}

void FindNextPoint(const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& input, int p, size_t& q,
                   double& min_angle, double& max_length) {
  min_angle = DBL_MAX;
  max_length = 0;
  for (size_t i = 0; i < input.size(); i++) {
    if (i != static_cast<size_t>(p)) {
      const auto angle = input[p].Angle(input[q], input[i]);
      const auto length = input[p].Length(input[i]);
      if (angle < min_angle || (angle == min_angle && length > max_length)) {
        q = i;
        min_angle = angle;
        max_length = length;
      }
    }
  }
}

}  // namespace

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::PreProcessingImpl() {
  if (task_data->inputs_count.empty() || task_data->inputs.empty() || !task_data->inputs[0]) {
    return false;
  }

  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<Vertex*>(task_data->inputs[0]);
  input_.assign(in_ptr, in_ptr + input_size);

  input_ = RemoveDuplicates(input_);

  unsigned int output_size = task_data->outputs_count[0];
  output_.reserve(output_size);

  rc_size_ = static_cast<int>(std::sqrt(input_size));
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::ValidationImpl() {
  if (task_data->inputs_count.size() != 1 || task_data->outputs_count.size() != 1 || task_data->inputs.size() != 1 ||
      task_data->outputs.size() != 1) {
    return false;
  }

  if (task_data->inputs_count[0] != task_data->outputs_count[0]) {
    return false;
  }

  if (task_data->inputs_count[0] > 0 && task_data->inputs[0] == nullptr) {
    return false;
  }
  if (task_data->outputs_count[0] > 0 && task_data->outputs[0] == nullptr) {
    return false;
  }

  return true;
}

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::RunImpl() {
  if (input_.size() < 3) {
    output_.clear();
    if (!input_.empty()) {
      output_.insert(output_.end(), input_.begin(), input_.end());
    }
    return true;
  }

  output_.clear();
  output_.reserve(input_.size());

  int start_idx;
  Vertex start_point;

  oneapi::tbb::task_arena arena(1);
  arena.execute([&] {
    tbb::task_group tg;
    tg.run([&] { FindStartPoint(input_, start_idx, start_point); });
    tg.wait();
  });

  int p = start_idx;
  do {
    output_.push_back(input_[p]);

    size_t q = 0;
    double min_angle, max_length;

    arena.execute([&] {
      tbb::task_group tg;
      tg.run([&] { FindNextPoint(input_, p, q, min_angle, max_length); });
      tg.wait();
    });

    p = static_cast<int>(q);

  } while (p != start_idx && output_.size() < input_.size());

  return true;
}

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);
  size_t copy_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  if (copy_size > 0 && output_.data()) {
    std::copy(output_.begin(), output_.begin() + copy_size, out_ptr);
  }

  return true;
}