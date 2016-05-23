#include "spec.h"
#include "gtest/gtest.h"
#include <cstddef>

std::string test_dir = "../../../test";
runcpp::spec::Spec spec(test_dir);

TEST(test_spec, test_spec_version) { EXPECT_EQ("0.5.0-dev", spec.oci_version); }

TEST(test_spec, test_spec_platform_os) { EXPECT_EQ("linux", spec.platform.os); }

TEST(test_spec, test_spec_root_path) { EXPECT_EQ("rootfs", spec.root.path); }
