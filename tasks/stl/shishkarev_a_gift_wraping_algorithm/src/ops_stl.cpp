#include "stl/shishkarev_a_gift_wraping_algorithm/include/ops_stl.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <execution>
#include <numeric>
#include <set>
#include <thread>
#include <vector>

#include "core/util/include/util.hpp"

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
  output_.reserve(output_size);

  thread_results_.resize(ppc::util::GetPPCNumThreads());
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
  output_.reserve(input_.size());

  auto start_point =
      std::min_element(input_.begin(), input_.end(),
                       [](const Vertex& a, const Vertex& b) { return a.x < b.x || (a.x == b.x && a.y < b.y); }) -
      input_.begin();

  const int num_threads = ppc::util::GetPPCNumThreads();
  std::vector<std::thread> threads(num_threads);
  std::vector<size_t> thread_results_(num_threads);

  int p = start_point;
  do {
    output_.push_back(input_[p]);
    size_t q = (p + 1) % input_.size();

    const size_t chunk_size = input_.size() / num_threads;

    for (int t = 0; t < num_threads; ++t) {
      size_t start = t * chunk_size;
      size_t end = (t == num_threads - 1) ? input_.size() : (t + 1) * chunk_size;

      threads[t] = std::thread([this, start, end, p, q, t, &thread_results_]() {
        size_t local_q = q;
        for (size_t i = start; i < end; i++) {
          const auto angle = input_[p].Angle(input_[local_q], input_[i]);
          if (angle < 0 || (angle == 0 && input_[p].Length(input_[i]) > input_[p].Length(input_[local_q]))) {
            local_q = i;
          }
        }
        thread_results_[t] = local_q;
      });
    }

    for (auto& thread : threads) {
      thread.join();
    }

    q = thread_results_[0];
    for (int t = 1; t < num_threads; ++t) {
      const auto angle = input_[p].Angle(input_[q], input_[thread_results_[t]]);
      if (angle < 0 || (angle == 0 && input_[p].Length(input_[thread_results_[t]]) > input_[p].Length(input_[q]))) {
        q = thread_results_[t];
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
