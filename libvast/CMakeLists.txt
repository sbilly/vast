configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vast/config.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/vast/config.hpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(libvast_sources
  ${aux_sources}
  src/address.cpp
  src/banner.cpp
  src/bitvector.cpp
  src/bitstream.cpp
  src/bitstream_polymorphic.cpp
  src/chunk.cpp
  src/cleanup.cpp
  src/compression.cpp
  src/data.cpp
  src/die.cpp
  src/expression.cpp
  src/error.cpp
  src/event.cpp
  src/filesystem.cpp
  src/http.cpp
  src/individual.cpp
  src/logger.cpp
  src/operator.cpp
  src/pattern.cpp
  src/port.cpp
  src/schema.cpp
  src/streambuf.cpp
  src/subnet.cpp
  src/system.cpp
  src/time.cpp
  src/type.cpp
  src/uuid.cpp
  src/value.cpp
  # FIXME: Bring back after having repaired CAF code.
  #src/actor/accountant.cpp
  #src/actor/archive.cpp
  #src/actor/exporter.cpp
  #src/actor/importer.cpp
  #src/actor/identifier.cpp
  #src/actor/index.cpp
  #src/actor/key_value_store.cpp
  #src/actor/node.cpp
  #src/actor/partition.cpp
  #src/actor/signal_monitor.cpp
  #src/actor/task.cpp
  #src/actor/sink/ascii.cpp
  #src/actor/sink/bro.cpp
  #src/actor/sink/csv.cpp
  #src/actor/sink/json.cpp
  #src/actor/sink/spawn.cpp
  #src/actor/source/bro.cpp
  #src/actor/source/bgpdump.cpp
  #src/actor/source/spawn.cpp
  #src/actor/source/test.cpp
  src/concept/convertible/vast/address.cpp
  src/concept/convertible/vast/data.cpp
  src/concept/convertible/vast/event.cpp
  src/concept/convertible/vast/pattern.cpp
  src/concept/convertible/vast/port.cpp
  src/concept/convertible/vast/schema.cpp
  src/concept/convertible/vast/subnet.cpp
  src/concept/convertible/vast/time.cpp
  src/concept/convertible/vast/type.cpp
  src/concept/convertible/vast/value.cpp
  src/concept/serializable/vast/expression.cpp
  src/concept/serializable/vast/vector_event.cpp
  src/detail/adjust_resource_consumption.cpp
  src/expr/evaluator.cpp
  src/expr/normalize.cpp
  src/expr/predicatizer.cpp
  src/expr/restrictor.cpp
  src/expr/resolver.cpp
  src/expr/validator.cpp
  src/util/fdistream.cpp
  src/util/fdinbuf.cpp
  src/util/fdostream.cpp
  src/util/fdoutbuf.cpp
  src/util/posix.cpp
  src/util/string.cpp
  src/util/system.cpp
  src/util/terminal.cpp
  src/util/detail/posix.cpp
  src/util/hash/crc.cpp
  src/util/hash/murmur.cpp
  src/util/hash/xxhash.cpp
)

# FIXME: Bring back after having repaired CAF code.
#if (GPERFTOOLS_FOUND)
#  set(libvast_sources ${libvast_sources}
#    src/actor/profiler.cpp)
#endif ()
#
#if (PCAP_FOUND)
#  set(libvast_sources ${libvast_sources}
#    src/actor/sink/pcap.cpp
#    src/actor/source/pcap.cpp)
#endif ()

set(libvast_libs ${CAF_LIBRARIES})

if (VAST_ENABLE_ASSERTIONS)
  set(libvast_libs ${libvast_libs} ${Backtrace_LIBRARIES})
endif ()

if (SNAPPY_FOUND)
  set(libvast_libs ${libvast_libs} ${SNAPPY_LIBRARIES})
endif ()

if (PCAP_FOUND)
  set(libvast_libs ${libvast_libs} ${PCAP_LIBRARIES})
endif ()

if (BROCCOLI_FOUND)
  set(libvast_libs ${libvast_libs} ${BROCCOLI_LIBRARIES})
endif ()

# Always link with -lprofile if we have Gperftools.
if (GPERFTOOLS_FOUND)
  set(libvast_libs ${libvast_libs} ${GPERFTOOLS_PROFILER})
endif ()

# Only link against tcmalloc if requested.
if (GPERFTOOLS_FOUND AND VAST_USE_PERFTOOLS_HEAP_PROFILER)
  set(libvast_libs ${libvast_libs} ${GPERFTOOLS_TCMALLOC})
endif ()

add_library(libvast SHARED ${libvast_sources})
set_target_properties(libvast
  PROPERTIES
  SOVERSION ${VERSION_MAJOR}
  VERSION ${VERSION_MAJOR}.${VERSION_MINOR}
  OUTPUT_NAME vast)

target_link_libraries(libvast ${libvast_libs})

# Install libvast in PREFIX/lib and headers in PREFIX/include/vast.
install(TARGETS libvast DESTINATION lib)
install(DIRECTORY vast/
        DESTINATION include/vast FILES_MATCHING PATTERN "*.hpp")

# ----------------------------------------------------------------------------
#                                 Unit Tests
# ----------------------------------------------------------------------------

# Bro logs.
set (test_data "#ifndef TEST_DATA\n#define TEST_DATA")
file(GLOB log_dirs test/logs/bro/*)
foreach (log_dir ${log_dirs})
  get_filename_component(log_dir_basename ${log_dir} NAME_WE)
  set(test_data "${test_data}\n\nnamespace ${log_dir_basename} {\n")
  file(GLOB logs ${log_dir}/*.log)
  foreach (log ${logs})
      get_filename_component(log_basename ${log} NAME_WE)
      set (this "constexpr auto ${log_basename} = \"${log}\";")
      set (test_data "${test_data}\n${this}")
  endforeach ()
  set(test_data "${test_data}\n\n} // namespace ${log_dir_basename}")
endforeach ()

# Bgpdump logs.
set(test_data "${test_data}\n\nnamespace bgpdump {\n")
file(GLOB logs test/logs/bgpdump/*.txt)
foreach (log ${logs})
  get_filename_component(log_basename ${log} NAME_WE)
  set (this "constexpr auto ${log_basename} = \"${log}\";")
  set (test_data "${test_data}\n${this}")
endforeach ()
set(test_data "${test_data}\n\n} // namespace bgpdump")

# PCAP traces.
set(test_data "${test_data}\n\nnamespace traces {\n")
file(GLOB traces test/traces/*.pcap)
foreach (trace ${traces})
  get_filename_component(trace_basename ${trace} NAME_WE)
  set(this "constexpr auto ${trace_basename} = \"${trace}\";")
  set(test_data "${test_data}\n${this}")
endforeach ()
set(test_data "${test_data}\n\n} // namespace traces")
set(test_data "${test_data}\n\n#endif")

configure_file(test/data.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test/data.hpp)

include_directories(
  ${CMAKE_SOURCE_DIR}/libvast
  ${CMAKE_BINARY_DIR}/libvast
  ${CMAKE_CURRENT_SOURCE_DIR}/test
  ${CMAKE_CURRENT_BINARY_DIR}/test)

# FIXME: Bring back after having repaired CAF code.
set(tests
  #test/actor/export.cpp
  #test/actor/import.cpp
  #test/actor/index.cpp
  #test/actor/indexer.cpp
  #test/actor/io.cpp
  #test/actor/key_value_store.cpp
  #test/actor/partition.cpp
  #test/actor/source_bgpdump.cpp
  #test/actor/task.cpp
  test/binner.cpp
  test/bitmap.cpp
  test/bitmap_index.cpp
  test/bitstream.cpp
  test/bitvector.cpp
  test/cache.cpp
  test/chunk.cpp
  test/data.cpp
  test/event.cpp
  test/expr.cpp
  test/filesystem.cpp
  test/hash.cpp
  test/intrusive.cpp
  test/iterator.cpp
  test/json.cpp
  test/logging.cpp
  test/main.cpp
  test/maybe.cpp
  test/offset.cpp
  test/parse_bro.cpp
  test/parse_data.cpp
  test/parse_expr.cpp
  test/parse_type.cpp
  test/parse_vast.cpp
  test/parseable.cpp
  test/print_vast.cpp
  test/printable.cpp
  test/range_map.cpp
  test/save_load.cpp
  test/schema.cpp
  test/search.cpp
  test/serialization.cpp
  test/stack.cpp
  test/streambuf.cpp
  test/string.cpp
  test/type.cpp
  test/uuid.cpp
  test/value.cpp
  test/variant.cpp
  test/detail/flat_serial_set.cpp
)

# FIXME: Bring back after having repaired CAF code.
#if (PCAP_FOUND)
#  set(tests ${tests}
#    test/actor/source_pcap.cpp)
#endif ()

add_executable(vast-test ${tests})
target_link_libraries(vast-test libvast ${CMAKE_THREAD_LIBS_INIT})

#
# Generate CMake tests
#

# Helper macro to construct a CMake test from a VAST test suite.
macro(make_test suite)
  string(REPLACE " " "_" test_name ${suite})
  set(vast_test ${EXECUTABLE_OUTPUT_PATH}/vast-test)
  add_test(${test_name} ${vast_test} -v 3 -s "${suite}" ${ARGN})
endmacro()

# Find all test suites.
foreach(test ${tests})
  file(STRINGS ${test} contents)
  foreach(line ${contents})
    if ("${line}" MATCHES "SUITE")
      string(REGEX REPLACE "#define SUITE \(.*\)" "\\1" suite ${line})
      list(APPEND suites ${suite})
    endif()
  endforeach()
endforeach()
list(REMOVE_DUPLICATES suites)

enable_testing()

foreach(suite ${suites})
  make_test("${suite}")
endforeach ()