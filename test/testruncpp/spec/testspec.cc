#include <cstddef>
#include "spec/spec.h"
#include "gtest/gtest.h"

std::string test_file = "test.json";
runcpp::spec::Spec spec(test_file);

TEST(test_spec, test_spec_version) { EXPECT_EQ("0.1", spec.version); }

TEST(test_spec, test_spec_platform_os) { EXPECT_EQ("linux", spec.platform.os); }

TEST(test_spec, test_spec_root_path) { EXPECT_EQ("rootfs", spec.root.path); }
