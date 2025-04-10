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

  rc_size_ = static_cast<int>(std::sqrt(input_size));
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::ValidationImpl() {
  return task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::RunImpl() {
  if (input_.size() < 3) {
    output_.assign(input_.begin(), input_.end());
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
  std::vector<Vertex> hull_points;

  do {
    hull_points.push_back(input_[p]);
    int q = -1;

    std::vector<int> local_qs;
    int initial_candidate = (p + 1) % static_cast<int>(input_.size());
    if (initial_candidate == p && input_.size() > 1) {
      initial_candidate = (p + 2) % static_cast<int>(input_.size());
    }
    if (input_.size() <= 1)
      initial_candidate = p;
    else if (initial_candidate == p)
      initial_candidate = (p > 0) ? 0 : 1;

#pragma omp parallel
    {
      int num_threads = omp_get_num_threads();
      int thread_id = omp_get_thread_num();

#pragma omp single
      { local_qs.resize(num_threads); }
      int thread_local_q = initial_candidate;

#pragma omp for nowait
      for (int i = 0; i < static_cast<int>(input_.size()); ++i) {
        if (i == p) continue;

        const auto angle = input_[p].Angle(input_[thread_local_q], input_[i]);

        if (angle < 0 || (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[thread_local_q]))) {
          thread_local_q = i;
        }
      }

      local_qs[thread_id] = thread_local_q;

#pragma omp barrier

#pragma omp single
      {
        q = local_qs[0];
        for (int k = 1; k < num_threads; ++k) {
          const auto angle = input_[p].Angle(input_[q], input_[local_qs[k]]);
          if (angle < 0 || (angle == 0 && input_[p].Length(input_[local_qs[k]]) > input_[p].Length(input_[q]))) {
            q = local_qs[k];
          }
        }
      }
    }

    if (q == -1 || q == p) {
      break;
    }

    p = q;
  } while (p != start_point);

  output_ = std::move(hull_points);
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_omp::TestTaskOpenMP::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);
  size_t copy_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  if (copy_size > 0) {
    if (out_ptr == nullptr) {
        return false;
    }
    std::copy(output_.begin(), output_.begin() + copy_size, out_ptr);
  }
  return true;
}
