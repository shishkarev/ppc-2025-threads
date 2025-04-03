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
  int input_size = static_cast<int>(task_data->inputs_count[0]);
  auto* in_ptr = reinterpret_cast<Vertex*>(task_data->inputs[0]);
  input_ = std::vector<Vertex>(in_ptr, in_ptr + input_size);

  input_ = RemoveDuplicates(input_);

  int output_size = static_cast<int>(task_data->outputs_count[0]);
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
    for (int i = 0; i < static_cast<int>(input_.size()); ++i) {
      output_.push_back(input_[i]);
    }
    return true;
  }

  output_.clear();

  int start_point = 0;
#pragma omp parallel
  {
    int local_start = start_point;
#pragma omp for nowait
    for (int i = 1; i < static_cast<int>(input_.size()); ++i) {
      if ((input_[i].y < input_[local_start].y) ||
          ((input_[i].y == input_[local_start].y) && (input_[i].x > input_[local_start].x))) {
        local_start = i;
      }
    }
#pragma omp critical
    {
      if ((input_[local_start].y < input_[start_point].y) ||
          ((input_[local_start].y == input_[start_point].y) && (input_[local_start].x > input_[start_point].x))) {
        start_point = local_start;
      }
    }
  }

  int p = start_point;
  do {
#pragma omp critical
    output_.push_back(input_[p]);  

    int q = (p + 1) % static_cast<int>(input_.size());

#pragma omp parallel
    {
      int local_q = q;
#pragma omp for nowait
      for (int i = 0; i < static_cast<int>(input_.size()); ++i) {
        const auto angle = input_[p].Angle(input_[q], input_[i]);
        if (angle < 0 || (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[q]))) {
          local_q = i;
        }
      }
#pragma omp critical
      {
        const auto angle = input_[p].Angle(input_[q], input_[local_q]);
        if (angle < 0 || (angle == 0 && input_[p].Length(input_[local_q]) > input_[p].Length(input_[q]))) {
          q = local_q;
        }
      }
    }
    p = q;
  } while (p != start_point);

  return true;
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);

  int min_size = std::min(static_cast<int>(output_.size()), static_cast<int>(task_data->outputs_count[0]));

  if (output_.size() >= min_size) {
    std::copy(output_.begin(), output_.begin() + min_size, out_ptr);
  }

  return true;
}
