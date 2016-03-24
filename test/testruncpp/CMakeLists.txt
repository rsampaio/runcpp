file(
  GLOB SRCS
  *.cc
  container/*.cc
  spec/*.cc
  ../../src/container/*.cc
  ../../src/spec/*.cc
)

include_directories(../../src)

add_executable(testruncpp ${SRCS})
#add_test(testruncpp testruncpp)

set_property(TARGET testruncpp PROPERTY CXX_STANDARD 11)

target_link_libraries(testruncpp libgtest libgmock stdc++fs)

install(TARGETS testruncpp DESTINATION bin)
