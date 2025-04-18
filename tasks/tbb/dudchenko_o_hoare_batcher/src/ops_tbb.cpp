#include "tbb/dudchenko_o_hoare_batcher/include/ops_tbb.hpp"

#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_invoke.h>
#include <tbb/blocked_range.h>

#include <algorithm>
#include <cmath>
#include <vector>

bool dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::PreProcessingImpl() {
  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<int*>(task_data->inputs[0]);
  input_ = std::vector<int>(in_ptr, in_ptr + input_size);

  unsigned int output_size = task_data->outputs_count[0];
  output_ = std::vector<int>(output_size, 0);

  return true;
}

bool dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::ValidationImpl() {
  if (task_data->inputs_count.empty() || task_data->outputs_count.empty() || task_data->inputs.empty() ||
      task_data->outputs.empty()) {
    return false;
  }

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

bool dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::RunImpl() {
  if (input_.empty()) {
    return false;
  }

  try {
    QuickSort(input_, 0, static_cast<int>(input_.size()) - 1);
    output_ = input_;
    return true;
  } catch (...) {
    return false;
  }
}

bool dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::PostProcessingImpl() {
  try {
    auto* out_ptr = reinterpret_cast<int*>(task_data->outputs[0]);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, output_.size()), [&](const tbb::blocked_range<size_t>& range) {
      for (size_t i = range.begin(); i < range.end(); i++) {
        out_ptr[i] = output_[i];
      }
    });
    return true;
  } catch (...) {
    return false;
  }
}

void dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::QuickSort(std::vector<int>& arr, int low, int high) {
  if (low >= high || low < 0 || high >= static_cast<int>(arr.size())) {
    return;
  }

  int mid = low + (high - low) / 2;
  if (arr[mid] < arr[low]) {
    std::swap(arr[mid], arr[low]);
  }
  if (arr[high] < arr[low]) {
    std::swap(arr[high], arr[low]);
  }
  if (arr[mid] < arr[high]) {
    std::swap(arr[mid], arr[high]);
  }

  int pi = Partition(arr, low, high);

  if (high - low > 10000) {
    tbb::parallel_invoke([&]() { QuickSort(arr, low, pi - 1); }, [&]() { QuickSort(arr, pi + 1, high); });
  } else {
    QuickSort(arr, low, pi - 1);
    QuickSort(arr, pi + 1, high);
  }
}

int dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::Partition(std::vector<int>& arr, int low, int high) {
  if (low >= high || low < 0 || high >= static_cast<int>(arr.size())) {
    return low;
  }

  int pivot = arr[high];
  int i = low - 1;

  for (int j = low; j <= high - 1; j++) {
    if (arr[j] <= pivot) {
      i++;
      if (i != j) {
        std::swap(arr[i], arr[j]);
      }
    }
  }
  if (i + 1 != high) {
    std::swap(arr[i + 1], arr[high]);
  }
  return i + 1;
}

void dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP::BatcherMerge(std::vector<int>& arr, int left, int mid, int right) {
  if (left >= right || left < 0 || right >= static_cast<int>(arr.size()) || mid < left || mid >= right) {
    return;
  }

  int n1 = mid - left + 1;
  int n2 = right - mid;

  std::vector<int> left_array(n1);
  std::vector<int> right_array(n2);

  tbb::parallel_invoke(
      [&]() {
        for (int i = 0; i < n1; i++) {
          left_array[i] = arr[left + i];
        }
      },
      [&]() {
        for (int j = 0; j < n2; j++) {
          right_array[j] = arr[mid + 1 + j];
        }
      });


  int i = 0;
  int j = 0;
  int k = left;

  while (i < n1 && j < n2) {
    if (left_array[i] <= right_array[j]) {
      arr[k] = left_array[i];
      i++;
    } else {
      arr[k] = right_array[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    arr[k] = left_array[i];
    i++;
    k++;
  }

  while (j < n2) {
    arr[k] = right_array[j];
    j++;
    k++;
  }
}