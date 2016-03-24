#include <cstddef>
#include "spec/spec.h"
#include "gtest/gtest.h"

TEST(test_spec, test_spec_parse) {
  std::string test_file = "test.json";
  runcpp::spec::Spec spec(test_file);

  EXPECT_EQ("0.1", spec.version);
}
