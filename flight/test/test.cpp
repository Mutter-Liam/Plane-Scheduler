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

// test correct init accounts and counts, test will be rewritten.
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
  string logs[3]{LANDING_MSG(0, 1, 0, 0, 90, 10, 10),
                 TAKEOFF_MSG(0, 2, 0, 0, 90, 10, 20),
                 LANDING_MSG(0, 3, 0, 0, 90, 10, 40)};

  int i = 0;
  airport_t = new Airport(2);

  // capture out
  stringstream output;
  streambuf *oldCoutStreamBuf = cout.rdbuf();  // save cout's streambuf
  cout.rdbuf(output.rdbuf());                  // redirect cout to stringstream

  airport_t->landing(0, 1, 90, 0, 10, 10, 10);
  airport_t->takeoff(0, 2, 90, 0, 20, 10, 20);
  airport_t->landing(0, 3, 90, 0, 30, 10, 40);
  
  cout.rdbuf(oldCoutStreamBuf);  // restore cout's original streambuf

  // compare the logs
  string line = "";
  while (getline(output, line)) {
    EXPECT_EQ(line, logs[i++]) << "Your log msg did not match the expected msg";
  }

  EXPECT_EQ(i, 3) << "There should be 3 lines in the log";
}

TEST(ScheduleTest, LoadScheduleTest){
  int res = load_schedule("test/examples/example1.txt");
  EXPECT_TRUE(res != -1) << "Load ledger did not load the ledger";

  int ids[]     = {1,  3, 4, 2};
  int fuels[]   = {9, 10, 20, 40};
  int times[]   = {5, 10, 30, 6};
  int r_times[] = {3, 10, 40, 8};
  int modes[]   = {1,  0, 1,  0};
  int i = 0;
  for (Schedule* item: schedule){
    EXPECT_EQ(item->flightID, ids[i]);
    EXPECT_EQ(item->fuelPercent, fuels[i]);
    EXPECT_EQ(item->scheduledTime, times[i]);
    EXPECT_EQ(item->timeSpentOnRunway, r_times[i]);
    EXPECT_EQ(item->requestTime, times[i]);
    EXPECT_EQ(item->mode, modes[i]);
    free(item);
    i++;
  }
}

TEST(SchedulingTest, SingleThreadTest){
  //Runs example2 with 1 producer and 1 consumer
  //HAVEN'T ADDED EXPECTED VALUES YET, JUST PRINTS RESULT

  string logs[4] = {
   "[ LANDING ] TID: 0 Flight: 1, ScheduledTime: 5, Runway: 0 Fuel: 9% LandingTime: 5 CompletionTime: 8",
   "[ TAKEOFF ] TID: 0 Flight: 3, ScheduledTime: 10, Runway: 0 Fuel: 10% TakeoffTime: 10 CompletionTime: 20",
   "[ LANDING ] TID: 0 Flight: 4, ScheduledTime: 30, Runway: 0 Fuel: 20% LandingTime: 30 CompletionTime: 70",
   "[ TAKEOFF ] TID: 0 Flight: 2, ScheduledTime: 6, Runway: 0 Fuel: 40% TakeoffTime: 20 CompletionTime: 28"
  };    
  // capture out
  fstream out("out.txt");
  streambuf *coutbuf = std::cout.rdbuf(); //save old buf
  cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

  //Run scheduling
  InitAirport(1, 1, 5, "test/examples/example1.txt", 0);

  cout.rdbuf(coutbuf);  // restore cout's original streambuf

  string line = "";
  int i = 0;
  getline(out, line);
  getline(out, line);
  getline(out, line);
  getline(out, line);
  getline(out, line);
  while (getline(out, line) && i < 4) {
    EXPECT_EQ(logs[i++], line);
  }
}

TEST(SchedulingTest, MultiThreadTest){
  //Runs example2 with 1 producer and 1 consumer
  //HAVEN'T ADDED EXPECTED VALUES YET, JUST PRINTS RESULT

  string logs[4] = {
    "[ LANDING ] TID: 0 Flight: 1, ScheduledTime: 5, Runway: 0 Fuel: 9% LandingTime: 5 CompletionTime: 8",
    "[ LANDING ] TID: 0 Flight: 4, ScheduledTime: 30, Runway: 0 Fuel: 20% LandingTime: 30 CompletionTime: 70",
    "[ TAKEOFF ] TID: 0 Flight: 2, ScheduledTime: 6, Runway: 0 Fuel: 40% TakeoffTime: 20 CompletionTime: 28",
    "[ TAKEOFF ] TID: 1 Flight: 3, ScheduledTime: 10, Runway: 1 Fuel: 10% TakeoffTime: 10 CompletionTime: 20"};
  // capture out

  fstream output("out.txt");
  streambuf *coutbuf = std::cout.rdbuf(); //save old buf
  cout.rdbuf(output.rdbuf()); //redirect std::cout to out.txt!             

  //Run scheduling
  InitAirport(2, 2, 5, "test/examples/example1.txt", 0);

  cout.rdbuf(coutbuf);  // restore cout's original streambuf
  string line = "";
  getline(output, line);
  getline(output, line);
  getline(output, line);
  getline(output, line);
  getline(output, line);
  int i = 0;
  while (getline(output, line) && i < 4) {
    EXPECT_TRUE(line.find(logs[i++]) != string::npos);
  }
}

TEST(SchedulingTest, CrashTest){
  //Runs example2 with 1 producer and 1 consumer
  //HAVEN'T ADDED EXPECTED VALUES YET, JUST PRINTS RESULT

  string logs[3] = {
    "[ LANDING ] TID: 0 Flight: 1, ScheduledTime: 0, Runway: 0 Fuel: 0% LandingTime: 0 CompletionTime: 10",
    "[ LANDING ] TID: 0 Flight: 2, ScheduledTime: 0, Runway: 1 Fuel: 0% LandingTime: 0 CompletionTime: 10",
    "[ LANDING ] TID: 0 Flight: 3, ScheduledTime: 0, Runway: 0 Fuel: 60% LandingTime: 10 CompletionTime: 20"
  }; 

 
  fstream output("out.txt");
  streambuf *coutbuf = std::cout.rdbuf();
  cout.rdbuf(output.rdbuf()); //redirect std::cout to out.txt!     
  //Run scheduling
  InitAirport(1, 1, 5, "test/examples/crash.txt", 0);

  cout.rdbuf(coutbuf);  // restore cout's original streambuf
  string line = "";
  getline(output, line);
  getline(output, line);
  getline(output, line);
  getline(output, line);
  getline(output, line);
  int i = 0;
  while (getline(output, line) && i < 3) {
    EXPECT_EQ(logs[i++], line);
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
