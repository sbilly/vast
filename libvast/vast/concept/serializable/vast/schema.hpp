#ifndef VAST_CONCEPT_SERIALIZABLE_VAST_SCHEMA_HPP
#define VAST_CONCEPT_SERIALIZABLE_VAST_SCHEMA_HPP

#include <iostream>

#include "vast/schema.hpp"
#include "vast/concept/serializable/std/string.hpp"
#include "vast/concept/parseable/parse.hpp"
#include "vast/concept/parseable/vast/schema.hpp"
#include "vast/concept/printable/to_string.hpp"
#include "vast/concept/printable/vast/schema.hpp"

namespace vast {

// TODO: we should figure out a better way to (de)serialize. Going through
// strings is not very efficient, although we currently have no other way to
// keep the pointer relationships of the types intact.

template <typename Serializer>
void serialize(Serializer& sink, schema const& sch) {
  sink << to_string(sch);
}

template <typename Deserializer>
void deserialize(Deserializer& source, schema& sch) {
  std::string str;
  source >> str;
  if (str.empty())
    return;
  sch.clear();
  auto i = str.begin();
  parse(i, str.end(), sch);
}

} // namespace vast

#endif
