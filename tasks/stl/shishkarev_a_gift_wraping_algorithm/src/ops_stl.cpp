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
  output_.reserve(output_size); // Reserve space instead of initializing with values

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
    output_.insert(output_.end(), input_.begin(), input_.end());
    return true;
  }

  output_.clear();
  output_.reserve(input_.size()); // Reserve space for worst case

  // Find leftmost point
  auto start_point = std::min_element(input_.begin(), input_.end(), 
    [](const Vertex& a, const Vertex& b) {
      return a.x < b.x || (a.x == b.x && a.y < b.y);
    }) - input_.begin();

  int p = start_point;
  do {
    output_.push_back(input_[p]);
    size_t q = (p + 1) % input_.size();

    for (size_t i = 0; i < input_.size(); i++) {
      const auto angle = input_[p].Angle(input_[q], input_[i]);
      if (angle < 0 || (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[q]))) {
        q = i;
      }
    }

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
