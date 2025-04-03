#include "omp/shishkarev_a_gift_wraping_algorithm/include/ops_omp.hpp"

#include <omp.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <vector>

std::vector<shishkarev_a_gift_wraping_algorithm_omp::Vertex> shishkarev_a_gift_wraping_algorithm_omp::RemoveDuplicates(
    const std::vector<shishkarev_a_gift_wraping_algorithm_omp::Vertex>& points) {
  std::set<Vertex> unique_points(points.begin(), points.end());
  return {unique_points.begin(), unique_points.end()};
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::PreProcessingImpl() {
  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<Vertex*>(task_data->inputs[0]);
  input_ = std::vector<Vertex>(in_ptr, in_ptr + input_size);

  input_ = RemoveDuplicates(input_);

  unsigned int output_size = task_data->outputs_count[0];
  output_ = std::vector<Vertex>(output_size, {0, 0});

  rc_size_ = static_cast<int>(std::sqrt(input_size));
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::ValidationImpl() {
  return task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::RunImpl() {
  if (input_.size() < 3) {
    output_.clear();
    for (size_t i = 1; i < input_.size(); ++i) {
      output_[i] = input_[i];
    }
    return true;
  }

  output_.clear();

  int start_point = 0;
#pragma omp parallel for reduction(min : start_point)
  for (size_t i = 1; i < input_.size(); ++i) {
    if ((input_[i].y < input_[start_point].y) ||
        ((input_[i].y == input_[start_point].y) && (input_[i].x > input_[start_point].x))) {
      start_point = static_cast<int>(i);
    }
  }

  int p = start_point;
  do {
    output_.push_back(input_[p]);
    size_t q = (p + 1) % input_.size();

#pragma omp parallel for
    for (size_t i = 0; i < input_.size(); ++i) {
      const auto angle = input_[p].Angle(input_[q], input_[i]);
      if (angle < 0 || (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[q]))) {
#pragma omp critical
        { q = i; }
      }
    }
    p = static_cast<int>(q);
  } while (p != start_point);

  return true;
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);

  size_t min_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  std::copy(output_.begin(), output_.begin() + min_size, out_ptr);

  return true;
}
