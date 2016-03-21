file(GLOB SRCS *.cc)

add_executable(testruncpp ${SRCS})

target_link_libraries(testruncpp libgtest libgmock)

install(TARGETS testruncpp DESTINATION bin)
