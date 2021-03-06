#include <cstring>
#include <vector>

#include "vast/util/string.hpp"

namespace vast {
namespace util {

std::string byte_escape(std::string const& str) {
  return escape(str, print_escaper);
}

std::string byte_escape(std::string const& str, std::string const& extra) {
  auto print_extra_escaper = [&](auto& f, auto l, auto out) {
    if (extra.find(*f) != std::string::npos) {
      *out++ = '\\';
      *out++ = *f++;
    } else {
      print_escaper(f, l, out);
    }
  };
  return escape(str, print_extra_escaper);
}

std::string byte_escape_all(std::string const& str) {
  return escape(str, hex_escaper);
}

std::string byte_unescape(std::string const& str) {
  return unescape(str, byte_unescaper);
}

std::string json_escape(std::string const& str) {
  if (str.empty())
    return "\"\"";
  std::string result;
  result.reserve(str.size() + 2);
  result += '"';
  auto f = str.begin();
  auto l = str.end();
  auto out = std::back_inserter(result);
  while (f != l)
    json_escaper(f, l, out);
  result += '"';
  return result;
}

std::string json_unescape(std::string const& str) {
  // Unescape everything until the closing double quote.
  auto f = str.begin();
  auto l = str.end();
  // Need at least two delimiting double quotes.
  if (f == l || l - f < 2)
    return {};
  // Only consider double-quoted strings.
  if (!(*f++ == '"' && (*--l == '"')))
    return {};
  std::string result;
  result.reserve(str.size());
  auto out = std::back_inserter(result);
  while (f != l)
    if (!json_unescaper(f, l, out))
      return {};
  return result;
}

std::string percent_escape(std::string const& str) {
  return escape(str, percent_escaper);
}

std::string percent_unescape(std::string const& str) {
  return unescape(str, percent_unescaper);
}

std::string double_escape(std::string const& str, std::string const& esc) {
  return escape(str, double_escaper(esc));
}

std::string double_unescape(std::string const& str, std::string const& esc) {
  return unescape(str, double_unescaper(esc));
}

std::string replace_all(std::string str, const std::string& search,
                        const std::string& replace) {
  auto pos = std::string::size_type{0};
  while ((pos = str.find(search, pos)) != std::string::npos) {
     str.replace(pos, search.length(), replace);
     pos += replace.length();
  }
  return str;
}

} // namespace util
} // namespace vast
