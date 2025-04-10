#include "omp/dudchenko_o_hoare_batcher/include/ops_omp.hpp"

#include <omp.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

bool dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::PreProcessingImpl() {
  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<int*>(task_data->inputs[0]);
  input_ = std::vector<int>(in_ptr, in_ptr + input_size);

  unsigned int output_size = task_data->outputs_count[0];
  output_ = std::vector<int>(output_size, 0);

  return true;
}

bool dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::ValidationImpl() {
  if (!task_data || (task_data->inputs[0] == nullptr) || (task_data->outputs[0] == nullptr)) {
    return false;
  }

  return task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::RunImpl() {
  if (input_.empty()) {
    return false;
  }

#pragma omp parallel
  {
#pragma omp single nowait
    { QuickSort(input_, 0, static_cast<int>(input_.size()) - 1); }
  }

  output_ = input_;
  return true;
}

bool dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::PostProcessingImpl() {
#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(output_.size()); i++) {
    reinterpret_cast<int*>(task_data->outputs[0])[i] = output_[i];
  }
  return true;
}

void dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::QuickSort(std::vector<int>& arr, int low, int high) {
  if (low < high) {
    int pi = Partition(arr, low, high);

#pragma omp parallel sections
    {
#pragma omp section
      { QuickSort(arr, low, pi - 1); }

#pragma omp section
      { QuickSort(arr, pi + 1, high); }
    }
  }
}

int dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::Partition(std::vector<int>& arr, int low, int high) {
  int pivot = arr[high];
  int i = (low - 1);

  for (int j = low; j <= high - 1; j++) {
    if (arr[j] < pivot) {
      i++;
      std::swap(arr[i], arr[j]);
    }
  }
  std::swap(arr[i + 1], arr[high]);
  return (i + 1);
}

void dudchenko_o_hoare_batcher_omp::TestTaskOpenMP::BatcherMerge(std::vector<int>& arr, int left, int mid, int right) {
  int n1 = mid - left + 1;
  int n2 = right - mid;

  std::vector<int> left_array(n1);
  std::vector<int> right_array(n2);

#pragma omp parallel sections
  {
#pragma omp section
    {
      for (int i = 0; i < n1; i++) {
        left_array[i] = arr[left + i];
      }
    }

#pragma omp section
    {
      for (int j = 0; j < n2; j++) {
        right_array[j] = arr[mid + 1 + j];
      }
    }
  }

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