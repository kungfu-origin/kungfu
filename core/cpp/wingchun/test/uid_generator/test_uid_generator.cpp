//
// Created by qlu on 2019/3/9.
//

#include "gtest/gtest.h"
#include "../uid_generator.h"

using namespace kungfu;

class UidGeneratorTest : public ::testing::Test {
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    UidGeneratorTest() {
        // You can do set-up work for each test here.
        uid_generator = new UidGenerator(1, 100000);
    }

    virtual ~UidGeneratorTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:
    virtual void SetUp() {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    virtual void TearDown() {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case for Project1.
    UidGenerator* uid_generator;
};

TEST_F(UidGeneratorTest, UniqueIDNotEqual)
{
    int i = 0;
    uid_generator->next_id(i);
    EXPECT_TRUE(false) << " custom failure message " << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
