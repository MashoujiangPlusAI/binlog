#include <binlog/ArrayView.hpp>

#include "test_utils.hpp"

#include <binlog/Session.hpp>
#include <binlog/SessionWriter.hpp>
#include <binlog/advanced_log_macros.hpp>

#include <doctest/doctest.h>

#include <vector>
#include <thread>
#include <string>

TEST_CASE("multi_thread")
{
  binlog::Session session;

  std::vector<std::thread> threads;
  uint64_t num_threads = std::thread::hardware_concurrency();
  threads.reserve(num_threads);
  static const std::string array[] = {"foo", "bar", "baz", "qux"};
  for (uint64_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&session, i](){
      // FIXME(Shoujiang): don't use thread_local in QNX
      static thread_local binlog::SessionWriter s_writer(
          session,
          1 << 20, // queue capacity
          0,       // writer id
          std::to_string(i) // writer name
      );
      BINLOG_INFO_W(s_writer, "Strings: {}", binlog::array_view(array, 4));
    });
  }

  std::vector<std::string> expected;
  expected.reserve(num_threads);
  for (uint64_t i = 0; i < num_threads; ++i) {
    expected.emplace_back("Strings: [foo, bar, baz, qux]");
  }

  for (auto&& thread: threads) {
    if(thread.joinable()) {
      thread.join();
    }
  }

  CHECK(getEvents(session, "%m") == expected);

}

TEST_CASE("multi_thread_sequence_check")
{
  binlog::Session session;

  std::vector<std::thread> threads;
  uint64_t num_threads = std::thread::hardware_concurrency();
//  uint64_t num_threads = 2;
  uint64_t num_loops {3};
  threads.reserve(num_threads);
  for (uint64_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&session, num_loops, i](){
      // FIXME(Shoujiang): don't use thread_local in QNX
      static thread_local binlog::SessionWriter s_writer(
          session,
          1 << 20, // queue capacity
          0,       // writer id
          std::to_string(i) // writer name
      );
      for (uint64_t j = 0; j < num_loops; ++j) {
        BINLOG_INFO_W(s_writer, "Thread {}: {}", i, j);
      }
    });
  }

  std::vector<std::string> expected;
  expected.reserve(num_threads);
  for (uint64_t i = 0; i < num_threads; ++i) {
    for (uint64_t j = 0; j < num_loops; ++j) {
      expected.emplace_back("Thread " + std::to_string(i) + ": " + std::to_string(j));
    }
  }
//  std::cout << "output size: " << expected.size() << std::endl;
  std::sort(expected.begin(), expected.end());
//  std::copy(expected.begin(), expected.end(), std::ostream_iterator<std::string>(std::cout, " "));
//  std::cout << std::endl;

  for (auto&& thread: threads) {
    if(thread.joinable()) {
      thread.join();
    }
  }

  auto output = getEvents(session, "%m");
//  std::cout << "output size: " << output.size() << std::endl;
  std::sort(output.begin(), output.end());
//  std::copy(output.begin(), output.end(), std::ostream_iterator<std::string>(std::cout, " "));
//  std::cout << std::endl;

  CHECK(output == expected);

}

