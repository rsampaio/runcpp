include(ExternalProject)

# docopt {
ExternalProject_Add(
  project_docopt
  GIT_REPOSITORY  "https://github.com/docopt/docopt.cpp.git"
  INSTALL_COMMAND ""
)

add_library(libdocopt_s STATIC IMPORTED)
ExternalProject_Get_Property(project_docopt binary_dir source_dir)
set_target_properties(libdocopt_s  PROPERTIES IMPORTED_LOCATION ${binary_dir}/libdocopt_s.a)
add_dependencies(libdocopt_s project_docopt)

include_directories(${source_dir})
# } docopt

# json {
ExternalProject_Add(
  json
  URL "https://github.com/nlohmann/json/archive/v1.1.0.tar.gz"
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
)
ExternalProject_Get_Property(json source_dir)
include_directories(${source_dir}/src)
# } json

# find sources        dir       variable
aux_source_directory( container container )
aux_source_directory( spec      spec      )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(runcpp runc.cc ${container} ${spec})

set_property(TARGET runcpp PROPERTY CXX_STANDARD 11)

target_link_libraries(runcpp libdocopt_s stdc++fs)

install(TARGETS runcpp DESTINATION bin)
