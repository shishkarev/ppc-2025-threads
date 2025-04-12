#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "core/task/include/task.hpp"
#include "core/util/include/util.hpp"
#include "tbb/shishkarev_a_gift_wraping_algorithm/include/ops_tbb.hpp"

TEST(shishkarev_a_gift_wraping_algorithm_tbb, test_convex_hull_simple) {
  constexpr size_t kCount = 10;

  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> in(kCount);
  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = {.x = rand() % 100, .y = rand() % 100};
  }

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  EXPECT_FALSE(out.empty());
  EXPECT_LE(out.size(), in.size());
}

TEST(shishkarev_a_gift_wraping_algorithm_tbb, test_convex_hull_medium) {
  constexpr size_t kCount = 1000;

  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> in(kCount);
  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = {.x = rand() % 100, .y = rand() % 100};
  }

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  EXPECT_FALSE(out.empty());
  EXPECT_LE(out.size(), in.size());
}

TEST(shishkarev_a_gift_wraping_algorithm_tbb, test_convex_hull_large) {
  constexpr size_t kCount = 100000;

  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> in(kCount);
  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = {.x = rand() % 100, .y = rand() % 100};
  }

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  EXPECT_FALSE(out.empty());
  EXPECT_LE(out.size(), in.size());
}

TEST(shishkarev_a_gift_wraping_algorithm_tbb, test_convex_hull_from_file) {
  std::ifstream test_file(ppc::util::GetAbsolutePath("tbb/shishkarev_a_gift_wraping_algorithm/data/test.txt"));

  size_t count = 0;
  test_file >> count;

  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> in(count);
  std::vector<shishkarev_a_gift_wraping_algorithm_tbb::Vertex> out(count);

  for (size_t i = 0; i < count; i++) {
    test_file >> in[i].x >> in[i].y;
  }
  test_file.close();

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  shishkarev_a_gift_wraping_algorithm_tbb::TestTaskTBB test_task_tbb(task_data_tbb);
  ASSERT_EQ(test_task_tbb.Validation(), true);
  test_task_tbb.PreProcessing();
  test_task_tbb.Run();
  test_task_tbb.PostProcessing();

  EXPECT_FALSE(out.empty());
  EXPECT_LE(out.size(), in.size());
}
