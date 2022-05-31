#ifndef BINLOG_ADAPT_PLUS_TYPE_HPP
#define BINLOG_ADAPT_PLUS_TYPE_HPP

// Make boost::filesystem components loggable by including this file

#include <binlog/adapt_enum.hpp>
#include <binlog/adapt_struct.hpp>

#include <mserialize/serialize.hpp>

#include <boost/filesystem.hpp>
#include <type_traits> // is_same

BINLOG_ADAPT_ENUM(boost::filesystem::file_type,
  status_error, file_not_found, regular_file, directory_file, symlink_file, block_file, character_file, fifo_file, socket_file, reparse_file, type_unknown
)

BINLOG_ADAPT_STRUCT(boost::filesystem::space_info, capacity, free, available)

namespace mserialize {

// BAD: p.string() allocates memory. It is called twice per logging
// on platforms where path::value_type is not char, e.g: Windows.
template <>
struct CustomSerializer<boost::filesystem::path>
{
  template <typename OutputStream>
  static void serialize(const boost::filesystem::path& p, OutputStream& ostream)
  {
      mserialize::serialize(p.string(), ostream);
  }

  static std::size_t serialized_size(const boost::filesystem::path& p)
  {
    return mserialize::serialized_size(p.string());
  }
};

template <>
struct CustomTag<boost::filesystem::path>
{
  static constexpr auto tag_string()
  {
    return make_cx_string("{boost::filesystem::path`str'[c}");
  }
};

} // namespace mserialize

BINLOG_ADAPT_STRUCT(boost::filesystem::directory_entry, path)

BINLOG_ADAPT_STRUCT(boost::system::error_code, message)
#endif // BINLOG_ADAPT_PLUS_TYPE_HPP
