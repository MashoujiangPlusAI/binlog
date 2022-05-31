#ifndef BINLOG_ADDRESS_HPP
#define BINLOG_ADDRESS_HPP

#include <binlog/adapt_struct.hpp>
#include <mserialize/detail/type_traits.hpp>

#include <memory>
#include <cstdint> // uintptr_t
#include <cstring> // memcpy

namespace binlog {

/**
 * Logging a pointer wrapped by this type causes bread
 * to show the address of the pointer in hex.
 *
 * Otherwise the pointed value would be shown.
 * When this object is logged, only the pointer
 * address is serialized, the pointed value is not.
 */
struct address
{
  explicit address(const void* pointer)
  {
    memcpy(&value, &pointer, sizeof(value));
  }

  // To make the binary representation platform agnostic,
  // we store the address in a u64, therefore a 64bit
  // bread can read a log produced by a 32bit program
  // without much complication.
  static_assert(sizeof(void*) <= sizeof(std::uint64_t), "");

  std::uint64_t value = 0;
};

} // namespace binlog

BINLOG_ADAPT_STRUCT(binlog::address, value)

// Log void* as address, without extra decoration

namespace mserialize {

template <>
struct CustomSerializer<void*>
{
  template <typename OutputStream>
  static void serialize(const void* pointer, OutputStream& ostream)
  {
    std::uint64_t value;
    memcpy(&value, &pointer, sizeof(value));
    mserialize::serialize(value, ostream);
  }

  static std::size_t serialized_size(const void*)
  {
    return sizeof(std::uint64_t);
  }
};

template <>
struct CustomSerializer<const void*> : CustomSerializer<void*> {};

template <>
struct CustomTag<void*> : CustomTag<binlog::address> {};

template <>
struct CustomTag<const void*> : CustomTag<binlog::address> {};

// Log all pointer as address, without extra decoration
template <typename T>
struct CustomSerializer<T, detail::enable_spec_if<std::is_pointer<T>>>: CustomSerializer<void*>{};

template <typename T>
struct CustomTag<T, detail::enable_spec_if<std::is_pointer<T>>> : CustomTag<binlog::address> {};


// Log shared_ptr as address, without extra decoration
template <typename T>
struct is_shared_ptr: std::false_type {};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>>: std::true_type {};

template <typename T>
struct CustomSerializer<T, detail::enable_spec_if<is_shared_ptr<T>>> {
  template <typename OutputStream>
  static void serialize(const T pointer, OutputStream& ostream)
  {
    std::uint64_t value;
    auto addr = pointer.get();
    memcpy(&value, &addr, sizeof(value));
    mserialize::serialize(value, ostream);
  }

  static std::size_t serialized_size(const T)
  {
    return sizeof(std::uint64_t);
  }
};
template <typename T>
struct CustomTag<T, detail::enable_spec_if<is_shared_ptr<T>>> : CustomTag<binlog::address> {};

} // namespace mserialize

#endif // BINLOG_ADDRESS_HPP
