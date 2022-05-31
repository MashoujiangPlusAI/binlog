#include <binlog/ArrayView.hpp>

#include "test_utils.hpp"

#include <binlog/Session.hpp>
#include <binlog/SessionWriter.hpp>
#include <binlog/advanced_log_macros.hpp>

#include <doctest/doctest.h>

#include <string>

struct StreamableObject {
  friend std::ostream& operator<<(std::ostream& os, const StreamableObject& obj) {
    os << "i_v: " << obj.i_v
       << ", d_v: " << obj.d_v
       << ", f_v: " << obj.f_v
       << ", b_v: " << obj.b_v
       << ", s_v: " << obj.s_v;
    return os;
  }
  int i_v = 88;
  double d_v {3.1415};
  float f_v {3};
  bool b_v = false;
  std::string s_v {"1234567890"};
};

TEST_CASE("streamable_object")
{
  binlog::Session session;
  binlog::SessionWriter writer(session, 128);

  StreamableObject object;
  BINLOG_INFO_W(writer, "streamable_object: {}", object);

  std::stringstream ss;
  ss << object;
  CHECK(getEvents(session, "%m") == std::vector<std::string>{"streamable_object: " + ss.str()});
}
