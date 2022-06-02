#ifndef BINLOG_DEFAULT_SESSION_HPP
#define BINLOG_DEFAULT_SESSION_HPP

#include <binlog/Session.hpp>
#include <binlog/SessionWriter.hpp>

#include <sstream>
#include <thread>

namespace binlog {

namespace detail {

/** Convert std::this_thread::get_id() to string */
inline std::string this_thread_id_string()
{
  std::ostringstream str;
  str << std::this_thread::get_id();
  return str.str();
}

} // namespace detail

/**
 * Get a global session.
 *
 * The session is shared with the whole program,
 * without any restrictions. This is useful
 * if different components do not want to
 * or cannot aggree on a common global session.
 *
 * This session is also used by basic
 * log macros, which hide the Session
 * concept from users.
 *
 * The implementation uses a function local static,
 * avoid using the returned reference in the context
 * of global destructors.
 */
inline Session& default_session()
{
  static Session s_session;
  return s_session;
}

/**
 * Get a thread-local writer for default_session().
 *
 * This writer is used by basic log macros.
 *
 * The implementation uses a function local static,
 * avoid using the returned reference in the context
 * of global destructors.
 */
inline SessionWriter& default_thread_local_writer()
{
#ifdef __QNX__
  static std::vector<SessionWriter> writers;
  static auto ret = [](){
    for(uint32_t i = 0; i < 40; ++i) {
      writers.emplace_back(default_session(),
                           1 << 20, // queue capacity
                           0,       // writer id
                           "");  // Will be updated by used thread
    }
    return true;
  }();
  (void)(ret);
  auto& thread_writer = writers.at(std::hash<std::thread::id>{}(std::this_thread::get_id())-1);
  if (thread_writer.getName().empty()) {
    thread_writer.setName(detail::this_thread_id_string()); // Update thread id
  }
  return thread_writer;
#else
  static thread_local SessionWriter s_writer(
      default_session(),
      1 << 20, // queue capacity
      0,       // writer id
      detail::this_thread_id_string() // writer name
      );
  return s_writer;
#endif
}

/**
 * Call default_session().consume.
 *
 * This simple shortcut makes sense if the Session
 * concept is hidden in the application, e.g:
 * if only basic log macros are used.
 *
 * @return description of the work done by consume
 */
template <typename OutputStream>
Session::ConsumeResult consume(OutputStream& out)
{
  return default_session().consume(out);
}

} // namespace binlog

#endif // BINLOG_DEFAULT_SESSION_HPP
