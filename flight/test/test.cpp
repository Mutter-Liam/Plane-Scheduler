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

#include "schedule.h"

using namespace std;
extern list<struct Schedule *> schedule;
Airport *airport_t;
sem_t glock;

// test correct init accounts and counts
TEST(AirportTest, TestAirportConstructor) {
  airport_t = new Airport(10);

  EXPECT_EQ(airport_t->getNum(), 10)
      << "Make sure to initialize runways to N";
  EXPECT_EQ(airport_t->getNumTakeoffs(), 0) << "Make sure to initialize num_takeoffs";
  EXPECT_EQ(airport_t->getNumLandings(), 0) << "Make sure to initialize num_landing";
  delete airport_t;
}

// make sure you pass this test case
TEST(Airport, TestLogs) {
  // expected logs ex
  string logs[5]{LANDING_MSG(0, 1, 0, 1, 80),
                 TAKEOFF_MSG(0, 2, 0, 2, 70),
                 LANDING_MSG(0, 3, 10, 1, 50)};

  int i = 0;
  airport_t = new Airport(2);

  // capture out
  stringstream output;
  streambuf *oldCoutStreamBuf = cout.rdbuf();  // save cout's streambuf
  cout.rdbuf(output.rdbuf());                  // redirect cout to stringstream

  airport_t->landing(0, 1, 90, 0, 10);
  airport_t->takeoff(0, 2, 90, 0, 20);
  airport_t->landing(0, 3, 90, 0, 30);
  
  cout.rdbuf(oldCoutStreamBuf);  // restore cout's original streambuf

  // compare the logs
  string line = "";
  while (getline(output, line)) {
    EXPECT_EQ(line, logs[i++]) << "Your log msg did not match the expected msg";
  }

  EXPECT_EQ(i, 3) << "There should be 5 lines in the log";
}

TEST(LedgerTest, LoadLedgerTest){
  int res = load_schedule("examples/example1.txt");
  EXPECT_TRUE(res != -1) << "Load ledger did not load the ledger";
  /*
  1 20 30 40 1
  2 4 6 8 0
  3 9 5 3 1
  10 10 10 10 0
  */
  int ids[]    = {1,  2, 3, 10};
  int fuels[]  = {20, 4, 9, 10};
  int times[]  = {30, 6, 5, 10};
  int r_times[] = {40, 0, 1, 10};
  int modes[]  = {1,  0, 1,  0};
  int i = 0;
  for (Schedule* item: schedule){
    EXPECT_EQ(item->flightID, ids[i]);
    EXPECT_EQ(item->fuelPercent, fuels[i]);
    EXPECT_EQ(item->scheduledTime, times[i]);
    EXPECT_EQ(item->timeSpentOnRunway, r_times[i]);
    EXPECT_EQ(item->mode, modes[i]);
    free(item);
    i++;
  }
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
