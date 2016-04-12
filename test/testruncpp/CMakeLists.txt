file(
  GLOB SRCS
  *.cc
  ../../src/container.cc
  ../../src/spec.cc
  ../../src/process.cc
)

include_directories(../../src/include)

add_executable(testruncpp ${SRCS})
#add_test(testruncpp testruncpp)

set_property(TARGET testruncpp PROPERTY CXX_STANDARD 14)

target_link_libraries(testruncpp libgtest libgmock stdc++fs)

install(TARGETS testruncpp DESTINATION bin)
