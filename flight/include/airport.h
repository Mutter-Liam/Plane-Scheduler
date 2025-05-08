#ifndef _AIRPORT_H
#define _AIRPORT_H

#include <assert.h> /* for assert */
#include <pthread.h>
#include <shared_mutex>
#include <semaphore.h> /* for sem */
#include <stdlib.h>    /* for atoi() and exit() */
#include <sys/wait.h>  /* for wait() */
#include <fstream>
#include <iostream> /* for cout */
#include <list>
#include <string>
#include <boundedBuffer.h>

using namespace std;



using namespace std;

#define TAKEOFF \
  std::string { "[ TAKEOFF ] " }
#define LANDING \
  std::string { "[ LANDING ] " }

#define TAKEOFF_MSG(tid, flightID ,timee, runway, fuel, actualTime, completionTime)                              \
  TAKEOFF + "TID: " + std::to_string(tid) + "Flight: " + std::to_string(flightID) + ", ScheduledTime: " + std::to_string(timee) + \
      ", Runway: " + std::to_string(runway) + " Fuel: " + std::to_string(fuel) + "%" + " TakeoffTime: " + std::to_string(actualTime) + " CompletionTime: " + std::to_string(completionTime)

#define LANDING_MSG(tid, flightID ,timee, runway, fuel, actualTime, completionTime)                               \
  LANDING + "TID: " + std::to_string(tid) + "F;ight: " + std::to_string(flightID) + ", ScheduledTime: " + std::to_string(timee) + \
      ", Runway: " + std::to_string(runway) + " Fuel: " + std::to_string(fuel) + "%" + " LandingTime: " + std::to_string(actualTime) + " CompletionTime: " + std::to_string(completionTime)

struct Runway {
  unsigned int runwayID;
  int takeoffs;
  int landings;
  int time;
  pthread_mutex_t lock;
};

class Airport {
 private:
  int num;
  int num_takeoffs;
  int num_landings;
  int *completion_times;
  int *response_times;

 public:
  Airport(int N);
  ~Airport();  // destructor

  int takeoff(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway, int actualTime, int completionTime);
  int landing(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway, int actualTime, int completionTime);


  // helper functions
  void print_runway();
  void recordTakeoff(string message, int runwayID);
  void recordLanding(string message, int runwayID);
  int getNum() { return num; }
  int getNumTakeoffs() { return num_takeoffs; }
  int getNumLandings() { return num_landings; }

  pthread_mutex_t airport_lock;
  pthread_cond_t runway_available_cond;
  struct Runway *runways;
  BoundedBuffer<struct Runway *> available_runways;
};

#endif