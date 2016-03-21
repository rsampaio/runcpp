file(GLOB SRCS *.cc container/*.cc)

add_executable(testruncpp ${SRCS})

target_link_libraries(testruncpp libgtest libgmock)

install(TARGETS testruncpp DESTINATION bin)
