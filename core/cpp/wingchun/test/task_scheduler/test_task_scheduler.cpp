//
// Created by PolarAir on 2019-03-12.
//

#include "gtest/gtest.h"
#include "../task_scheduler.h"

using namespace kungfu;

class TaskSchedulerTest : public ::testing::Test
{
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    TaskSchedulerTest() : task_scheduler(nullptr)
    {
        // You can do set-up work for each test here.
        task_scheduler = new TaskScheduler();
    }

    virtual ~TaskSchedulerTest()
    {
        // You can do clean-up work that doesn't throw exceptions here.
        delete(task_scheduler);
        task_scheduler = nullptr;
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:
    virtual void SetUp()
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
        task_scheduler->run();
    }

    virtual void TearDown()
    {
        // Code here will be called immediately after each test (right
        // before the destructor).
        task_scheduler->stop();
    }

    // Objects declared here can be used by all tests in the test case for Project1.
    TaskScheduler* task_scheduler;
};

TEST_F(TaskSchedulerTest, UniqueIDNotEqual)
{
    long n = 0;
    task_scheduler->update_nano(100);
    TSCallback lambda = [](long n) { ++n; };
    task_scheduler->insert_callback_at(200, lambda);
    task_scheduler->update_nano(200);
    EXPECT_TRUE(n == 1) << "task scheduler sync task call failed" << std::endl;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}