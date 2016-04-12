include(ExternalProject)

ExternalProject_Add(
  docopt
  GIT_REPOSITORY  "https://github.com/docopt/docopt.cpp.git"
  INSTALL_COMMAND ""
)

add_library(libdocopt_s STATIC IMPORTED)
ExternalProject_Get_Property(docopt binary_dir source_dir)
set_target_properties(libdocopt_s  PROPERTIES IMPORTED_LOCATION ${binary_dir}/libdocopt_s.a)
add_dependencies(libdocopt_s docopt)
include_directories(${source_dir})

ExternalProject_Add(
  json
  URL "https://github.com/nlohmann/json/archive/v1.1.0.tar.gz"
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
)
ExternalProject_Get_Property(json source_dir)
include_directories(${source_dir}/src)

ExternalProject_Add(
  easyloggingpp
  URL "https://github.com/easylogging/easyloggingpp/releases/download/v9.80/easyloggingpp_v9.80.tar.gz"
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
)
ExternalProject_Get_Property(easyloggingpp source_dir)
include_directories(${source_dir})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(runcpp runc.cc container.cc spec.cc process.cc)

set_property(TARGET runcpp PROPERTY CXX_STANDARD 14)

target_link_libraries(runcpp libdocopt_s stdc++fs)

install(TARGETS runcpp DESTINATION bin)
