#include <iostream>
#include <fstream>
#include <vector>

#include <binlog/binlog.hpp>

struct StreamableObject {
  friend std::ostream& operator<<(std::ostream& os, const StreamableObject& not_trivial) {
    os << not_trivial.m;
    return os;
  }
  static constexpr const char* VERSION_PATH = "/tmp/version-plus.txt";
  int m {1234};
};
constexpr const char* StreamableObject::VERSION_PATH; // Has to declare here!

struct NonStreamableObject {
  int m {5678};
};

typedef struct
{
  uint32_t msg_id;
}__attribute__((packed)) MessageHeaderType;

int main() {
  std::cout << __cplusplus << std::endl;
  int i = 88;
  double d = 66;
  float f = 77;
  bool b = false;
  std::string s {"1234567890"};
  std::vector<int> iv {1,3,6,7};
  auto int_ptr = std::make_shared<int>();
  const auto const_int_ptr = std::make_shared<int>();
  StreamableObject streamable_object;
  auto streamable_object_ptr = std::make_shared<StreamableObject>();
  auto obj_unique_ptr = std::make_unique<StreamableObject>();
  NonStreamableObject non_streamable_object;
  auto* non_streamable_object_raw_ptr = new NonStreamableObject();
  auto non_streamable_object_ptr = std::make_shared<NonStreamableObject>();

  binlog::Session session;
  binlog::SessionWriter writer(session, 128);
  std::uintptr_t address = 0xF777123;
  const int* const_any_pointer = nullptr;
  const void* const_void_pointer = nullptr;
  std::memcpy(&const_any_pointer, &address, sizeof(const_any_pointer));
  std::memcpy(&const_void_pointer, &address, sizeof(const_void_pointer));
  BINLOG_INFO("Raw pointer value: {}", const_any_pointer);

  // Basic type
  BINLOG_INFO("hello i: {}", i);
  BINLOG_INFO("hello: {}", d);
  BINLOG_INFO("hello: {}", f);
  BINLOG_INFO("hello: {}", b);
  BINLOG_INFO("hello: {}", iv);
  BINLOG_INFO("hello: {}", typeid(i).name());
  BINLOG_INFO("hello s: {}", s);
  BINLOG_INFO("hello s: {}", s.data());

  // New feature:
  // 1) streamable type
  // 2) always log raw pointer
  BINLOG_ERROR("{}", streamable_object);
  BINLOG_ERROR("{}", streamable_object_ptr);
  BINLOG_ERROR("{}", &streamable_object_ptr);
  BINLOG_ERROR("{}", streamable_object_ptr.get());
  BINLOG_ERROR("{}", static_cast<void*>(streamable_object_ptr.get()));
  BINLOG_ERROR("{}", *streamable_object_ptr);
  BINLOG_ERROR("{}", streamable_object_ptr->m);
//  BINLOG_ERROR("{}", non_streamable_object); // Not support, compile error
//  BINLOG_ERROR("{}", *non_streamable_object_ptr); // Not support, compile error
  BINLOG_ERROR("{}", non_streamable_object_ptr->m);
  BINLOG_ERROR("{}", non_streamable_object_raw_ptr);
  BINLOG_ERROR("{}", non_streamable_object_ptr.get());

  auto *msg_header_type = new MessageHeaderType;
  // Not support yet, compile error. More details: https://github.com/morganstanley/binlog/issues/144
//  BINLOG_ERROR("Failed to send local message {}", msg_header_type->msg_id);
  BINLOG_ERROR("Failed to send local message {}", static_cast<uint32_t>(msg_header_type->msg_id)); // Workaround

  // For static constexpr type, we have to declare, or use >c++17, or g++ 11.2.0: https://github.com/morganstanley/binlog/issues/143
  BINLOG_CRITICAL("VERSION_PATH: {}", StreamableObject::VERSION_PATH);

//  BINLOG_INFO("{}, {}", "hello"); // Compile error, mismatch
//  BINLOG_INFO("{}");  // Updata: Not support again
  BINLOG_INFO("hello world");
//  BINLOG_ERROR(s);  // Updata: Not support again
//  BINLOG_ERROR("Failed to load " + s + "/data.pb");  // Updata: Not support again

  BINLOG_INFO("{}", "world");
  BINLOG_INFO("{}", "{}");
  BINLOG_INFO("{}", "{} world");
  BINLOG_INFO("hello {}", "world");
  BINLOG_INFO("hello {}", s);

  // Only log address for all types of pointer
  BINLOG_INFO("hello ptr: {}", binlog::address(int_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", int_ptr);
  BINLOG_INFO("hello ptr metadata: {}", int_ptr.get());
  BINLOG_INFO("hello ptr metadata: {}", static_cast<void*>(int_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", static_cast<const void*>(int_ptr.get()));

  BINLOG_INFO("hello ptr: {}", binlog::address(const_int_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", const_int_ptr);
  BINLOG_INFO("hello ptr metadata: {}", const_int_ptr.get());
  BINLOG_INFO("hello ptr metadata: {}", static_cast<void*>(const_int_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", static_cast<const void*>(const_int_ptr.get()));

  BINLOG_INFO("hello ptr: {}", binlog::address(obj_unique_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", obj_unique_ptr);
  BINLOG_INFO("hello ptr metadata: {}", obj_unique_ptr.get());
  BINLOG_INFO("hello ptr metadata: {}", static_cast<void*>(obj_unique_ptr.get()));
  BINLOG_INFO("hello ptr metadata: {}", static_cast<const void*>(obj_unique_ptr.get()));

  // Mixed with other types
  BINLOG_INFO("hello not_trivial: {}, {}", i, streamable_object);
  BINLOG_INFO("hello not_trivial: {}, {}, {}, {}, {}, {}", i, d, f, b, s, streamable_object);

  // New feature
//  boost::filesystem::path test_file {__FILE__};
//  BINLOG_INFO(test_file);
//  BINLOG_INFO("{}", test_file);

  std::ofstream logfile("NewFeatureTest.blog", std::ofstream::out|std::ofstream::binary);
  binlog::consume(logfile);

  return 0;
}