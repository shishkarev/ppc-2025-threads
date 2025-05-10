#include "stl/shishkarev_a_gift_wraping_algorithm/include/ops_stl.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <execution>
#include <numeric>
#include <set>
#include <vector>

std::vector<shishkarev_a_gift_wraping_algorithm_stl::Vertex> shishkarev_a_gift_wraping_algorithm_stl::RemoveDuplicates(
    const std::vector<shishkarev_a_gift_wraping_algorithm_stl::Vertex>& points) {
  std::set<Vertex> unique_points(points.begin(), points.end());
  return {unique_points.begin(), unique_points.end()};
}

bool shishkarev_a_gift_wraping_algorithm_stl::TestTaskSTL::PreProcessingImpl() {
  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<Vertex*>(task_data->inputs[0]);
  input_ = std::vector<Vertex>(in_ptr, in_ptr + input_size);

  input_ = RemoveDuplicates(input_);

  unsigned int output_size = task_data->outputs_count[0];
  output_ = std::vector<Vertex>(output_size, {0, 0});

  rc_size_ = static_cast<int>(std::sqrt(input_size));
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_stl::TestTaskSTL::ValidationImpl() {
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

bool shishkarev_a_gift_wraping_algorithm_stl::TestTaskSTL::RunImpl() {
  if (input_.size() < 3) {
    output_.clear();
    for (size_t i = 1; i < input_.size(); ++i) {
      output_[i] = input_[i];
    }
    return true;
  }

  output_.clear();

  std::vector<int> start_points(input_.size());
  std::iota(start_points.begin(), start_points.end(), 0);

  int start_point = std::transform_reduce(
      start_points.begin(), start_points.end(), 0, [](int a, int b) { return std::min(a, b); },
      [this](int i) {
        return ((input_[i].y < input_[0].y) || ((input_[i].y == input_[0].y) && (input_[i].x > input_[0].x))) ? i : 0;
      });

  int p = start_point;
  do {
    output_.push_back(input_[p]);
    size_t q = (p + 1) % input_.size();

    std::vector<size_t> indices(input_.size());
    std::iota(indices.begin(), indices.end(), 0);

    q = std::transform_reduce(
        indices.begin(), indices.end(), q, [](size_t a, size_t b) { return std::min(a, b); },
        [this, p](size_t i) {
          const auto angle = input_[p].Angle(input_[(p + 1) % input_.size()], input_[i]);
          if (angle < 0 ||
              (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[(p + 1) % input_.size()]))) {
            return i;
          }
          return (p + 1) % input_.size();
        });

    p = static_cast<int>(q);
  } while (p != start_point);

  return true;
}

bool shishkarev_a_gift_wraping_algorithm_stl::TestTaskSTL::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);

  size_t min_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  std::copy(output_.begin(), output_.begin() + min_size, out_ptr);

  return true;
}
