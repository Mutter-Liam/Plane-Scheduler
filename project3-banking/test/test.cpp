#include <gtest/gtest.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "ledger.h"

using namespace std;

Bank *bank_t;
sem_t glock;

// test correct init accounts and counts
TEST(BankTest, TestBankConstructor) {
  bank_t = new Bank(10);

  EXPECT_EQ(bank_t->getNum(), 10)
      << "Make sure to initialize the account balance to 0";
  EXPECT_EQ(bank_t->getNumSucc(), 0) << "Make sure to initialize num_succ";
  EXPECT_EQ(bank_t->getNumFail(), 0) << "Make sure to initialize num_fail";
  delete bank_t;
}

// make sure you pass this test case
TEST(BankTest, TestLogs) {
  // expected logs ex
  string logs[5]{"[ SUCCESS ] TID: 0, LID: 0, Acc: 1 DEPOSIT $100",
                 "[ SUCCESS ] TID: 0, LID: 1, Acc: 1 WITHDRAW $50",
                 "[ SUCCESS ] TID: 0, LID: 2, Acc: 1 TRANSFER $10 TO Acc: 0",
                 "[ FAIL ] TID: 0, LID: 3, Acc: 3 WITHDRAW $100",
                 "[ FAIL ] TID: 0, LID: 4, Acc: 6 TRANSFER $200 TO Acc: 7"};

  int i = 0;
  bank_t = new Bank(10);

  // capture out
  stringstream output;
  streambuf *oldCoutStreamBuf = cout.rdbuf();  // save cout's streambuf
  cout.rdbuf(output.rdbuf());                  // redirect cout to stringstream

  // success msgs
  bank_t->deposit(0, 0, 1, 100);
  bank_t->withdraw(0, 1, 1, 50);
  bank_t->transfer(0, 2, 1, 0, 10);

  // fail msgs
  bank_t->withdraw(0, 3, 3, 100);
  bank_t->transfer(0, 4, 6, 7, 200);

  cout.rdbuf(oldCoutStreamBuf);  // restore cout's original streambuf

  // compare the logs
  string line = "";
  while (getline(output, line)) {
    EXPECT_EQ(line, logs[i++]) << "Your log msg did not match the expected msg";
  }

  EXPECT_EQ(i, 5) << "There should be 5 lines in the log";
}

TEST(PCTest, Test1) {
  BoundedBuffer<int> *BB = new BoundedBuffer<int>(5);
  EXPECT_TRUE(BB->isEmpty());

  delete BB;
}

// Test checking append() and remove() from buffer
TEST(PCTest, Test2) {
  BoundedBuffer<int> *BB = new BoundedBuffer<int>(5);
  BB->append(0);
  ASSERT_EQ(0, BB->remove());

  delete BB;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
