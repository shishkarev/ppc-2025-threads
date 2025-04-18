#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "core/task/include/task.hpp"
#include "core/util/include/util.hpp"
#include "tbb/dudchenko_o_hoare_batcher/include/ops_tbb.hpp"

TEST(dudchenko_o_hoare_batcher_tbb, test_sort_50) {
  constexpr size_t kCount = 50;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  for (size_t i = 0; i < kCount; ++i) {
    in[i] = std::rand() % 1000;
  }

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  std::vector<int> sorted_in = in;
  std::ranges::sort(sorted_in);
  EXPECT_EQ(sorted_in, out);
}

TEST(dudchenko_o_hoare_batcher_tbb, test_sort_100_from_file) {
  std::string line;
  std::ifstream test_file(ppc::util::GetAbsolutePath("tbb/dudchenko_o_hoare_batcher/data/test.txt"));
  if (test_file.is_open()) {
    getline(test_file, line);
  }
  test_file.close();

  const size_t count = std::stoi(line);

  std::vector<int> in(count);
  std::vector<int> out(count);

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  for (size_t i = 0; i < count; ++i) {
    in[i] = std::rand() % 1000;
  }

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t *>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t *>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  dudchenko_o_hoare_batcher_tbb::TestTaskOpenMP test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  std::vector<int> sorted_in = in;
  std::ranges::sort(sorted_in);
  EXPECT_EQ(sorted_in, out);
}