#ifndef _BANK_H
#define _BANK_H

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

using namespace std;

#define TAKEOFF_MSG(level, w, l, a, m)                                 \
  level + "TID: " + std::to_string(w) + ", LID: " + std::to_string(l) + \
      ", Acc: " + std::to_string(a) + " DEPOSIT $" + std::to_string(m)

#define LANDING_MSG(level, w, l, a, m)                                 \
  level + "TID: " + std::to_string(w) + ", LID: " + std::to_string(l) + \
      ", Acc: " + std::to_string(a) + " WITHDRAW $" + std::to_string(m)

using namespace std;

#define TAKEOFF \
  std::string { "[ TAKEOFF ] " }
#define LANDING \
  std::string { "[ LANDING ] " }

struct Runway {
  unsigned int runwayID;
  int takeoffs;
  int landings;
  pthread_mutex_t lock;
};

class Airport {
 private:
  int num;
  int num_takeoffs;
  int num_landings;

 public:
  Airport(int N);
  ~Airport();  // destructor

  int takeoff(int workerID, int ledgerID, int accountID, int amount);
  int landing(int workerID, int ledgerID, int accountID, int amount);


  // helper functions
  void print_runway();
  void recordTakeoff(string message);
  void recordLanding(string message);
  int getNum() { return num; }
  int getNumTakeoffs() { return num_takeoffs; }
  int getNumLandings() { return num_landings; }

  pthread_mutex_t airport_lock;
  struct Runway *runways;
};

#endif