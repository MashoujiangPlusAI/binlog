#include <binlog/ArrayView.hpp>

#include "test_utils.hpp"
#include "binlog/Address.hpp"

#include <binlog/Session.hpp>
#include <binlog/SessionWriter.hpp>
#include <binlog/advanced_log_macros.hpp>

#include <doctest/doctest.h>

#include <string>

struct ForTest{
  int data {12};
};

TEST_CASE("null_pointer")
{
  binlog::Session session;
  binlog::SessionWriter writer(session, 128);

  int* ptr = nullptr;
  BINLOG_INFO_W(writer, "Pointers: {}", ptr);
  CHECK(getEvents(session, "%m") == std::vector<std::string>{"Pointers: 0x0"});
}

TEST_CASE("raw_pointer")
{
  binlog::Session session;
  binlog::SessionWriter writer(session, 128);

  // Addresses (raw pointer value)
  std::uintptr_t address = 0xF777123;

  int* any_pointer = nullptr;
  void* void_pointer = nullptr;
  std::memcpy(&any_pointer, &address, sizeof(any_pointer));
  std::memcpy(&void_pointer, &address, sizeof(void_pointer));
  BINLOG_INFO_W(writer, "Raw pointer value: {} {} {}", binlog::address(any_pointer), any_pointer, void_pointer);
  CHECK(getEvents(session, "%m") == std::vector<std::string>{"Raw pointer value: 0xF777123 0xF777123 0xF777123"});
}

TEST_CASE("const_raw_pointer")
{
  binlog::Session session;
  binlog::SessionWriter writer(session, 128);

  // Addresses (raw pointer value)
  std::uintptr_t address = 0xF777123;

  const ForTest* const_any_pointer = nullptr;
  const void* const_void_pointer = nullptr;
  std::memcpy(&const_any_pointer, &address, sizeof(const_any_pointer));
  std::memcpy(&const_void_pointer, &address, sizeof(const_void_pointer));
  BINLOG_INFO_W(writer, "Raw pointer value: {} {} {}", binlog::address(const_any_pointer), const_any_pointer, const_void_pointer);
  CHECK(getEvents(session, "%m") == std::vector<std::string>{"Raw pointer value: 0xF777123 0xF777123 0xF777123"});
}

TEST_CASE("smart_pointer")
{
  binlog::Session session;
  binlog::SessionWriter writer(session, 128);

  std::unique_ptr<ForTest> uptr = std::make_unique<ForTest>();
  std::shared_ptr<ForTest> sptr = std::make_shared<ForTest>();
  uptr.reset();
  sptr.reset();
  BINLOG_INFO_W(writer, "Smart pointer value: {} {}", uptr, sptr);
  CHECK(getEvents(session, "%m") == std::vector<std::string>{"Smart pointer value: 0x0 0x0"});

  // weak_ptr is not loggable by design, must be .lock()-ed first
  static_assert(
      !mserialize::detail::is_serializable<std::weak_ptr<int>>::value,
      "std::weak_ptr is not loggable"
  );
}