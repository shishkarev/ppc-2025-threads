#include "tbb/shishkarev_a_gift_wraping_algorithm/include/ops_tbb.hpp"

#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/tbb.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <set>
#include <utility>
#include <vector>

namespace {

int FindStartPoint(const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& input) {
  int start_point = 0;
  tbb::parallel_for(tbb::blocked_range<size_t>(1, input.size()), [&](const tbb::blocked_range<size_t>& range) {
    int local_start = start_point;
    for (size_t i = range.begin(); i < range.end(); ++i) {
      if ((input[i].y < input[local_start].y) ||
          ((input[i].y == input[local_start].y) && (input[i].x > input[local_start].x))) {
        local_start = static_cast<int>(i);
      }
    }
    tbb::mutex mutex;
    tbb::mutex::scoped_lock lock(mutex);
    if ((input[local_start].y < input[start_point].y) ||
        ((input[local_start].y == input[start_point].y) && (input[local_start].x > input[start_point].x))) {
      start_point = local_start;
    }
  });
  return start_point;
}

int GetInitialCandidate(const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& input, int p) {
  if (input.size() <= 1) {
    return p;
  }

  int candidate = (p + 1) % static_cast<int>(input.size());
  if (candidate != p) {
    return candidate;
  }

  return (p > 0) ? 0 : 1;
}

bool IsBetterCandidate(const shishkarev_a_gift_wraping_algorithm_tbb::Vertex& p_vertex,
                       const shishkarev_a_gift_wraping_algorithm_tbb::Vertex& current,
                       const shishkarev_a_gift_wraping_algorithm_tbb::Vertex& candidate) {
  const auto angle = p_vertex.Angle(current, candidate);
  return angle < 0 || (angle == 0 && p_vertex.Length(candidate) > p_vertex.Length(current));
}

int FindNextPoint(const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& input, int p) {
  int initial_candidate = 0;
  initial_candidate = GetInitialCandidate(input, p);

  struct ThreadData {
    int q{-1};
    ThreadData() = default;
  };
  tbb::enumerable_thread_specific<ThreadData> tls;

  tbb::parallel_for(tbb::blocked_range<size_t>(0, input.size()), [&](const tbb::blocked_range<size_t>& range) {
    ThreadData& local = tls.local();
    if (local.q == -1) {
      local.q = initial_candidate;
    }

    for (size_t i = range.begin(); i < range.end(); ++i) {
      if (static_cast<int>(i) == p) {
        continue;
      }
      if (IsBetterCandidate(input[p], input[local.q], input[i])) {
        local.q = static_cast<int>(i);
      }
    }
  });

  int q = initial_candidate;
  for (const auto& thread_data : tls) {
    if (thread_data.q != -1 && IsBetterCandidate(input[p], input[q], input[thread_data.q])) {
      q = thread_data.q;
    }
  }

  return q;
}

std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> RemoveDuplicates(
    const std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>& points) {
  std::set<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> unique_points(points.begin(), points.end());
  return std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex>(unique_points.begin(), unique_points.end());
}

}  // namespace

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::PreProcessingImpl() {
  int input_size = static_cast<int>(task_data->inputs_count[0]);
  auto* in_ptr = reinterpret_cast<Vertex*>(task_data->inputs[0]);
  input_ = std::vector<Vertex>(in_ptr, in_ptr + input_size);

  input_ = RemoveDuplicates(input_);

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

  int start_point = 0;
  start_point = FindStartPoint(input_);
  int p = start_point;
  std::vector<Vertex> hull_points;
  do {
    hull_points.push_back(input_[p]);
    int q = 0;
    q = FindNextPoint(input_, p);
    if (q == -1 || q == p) {
      break;
    }
    p = q;
  } while (p != start_point);

  output_ = std::move(hull_points);
  return true;
}

bool shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB::PostProcessingImpl() {
  auto* out_ptr = reinterpret_cast<Vertex*>(task_data->outputs[0]);
  size_t copy_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  if (copy_size > 0) {
    std::memcpy(out_ptr, output_.data(), copy_size * sizeof(Vertex));
  }
  return true;
}