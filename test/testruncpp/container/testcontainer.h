#include "gtest/gtest.h"

class TestContainer : public ::testing::Test {
 protected:
    TestContainer();
    virtual ~TestContainer();
};
