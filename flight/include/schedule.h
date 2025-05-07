#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include <airport.h>
#include <boundedBuffer.h>
#include <algorithm>

#ifdef DEBUGMODE
#define debug(msg) \
  std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << msg << std::endl;
#else
#define debug(msg)
#endif

using namespace std;

#define T 0
#define L 1

const int SEED_RANDOM = 377;

struct Schedule {
  int flightID;
  int fuelPercent;
  int scheduledTime;
  int timeSpentOnRunway;
  int requestTime;
  int completionTime;
  int mode;
};

extern list<struct Schedule*> schedule;
extern Airport *airport;
extern BoundedBuffer<struct Schedule*> *bb; 
extern int max_items;
extern int con_items;

void InitAirport(int np, int nc, int size, char *filename);
int load_schedule(char *filename);
void *consumer(void *workerID);
void *producer(void *unused);

#endif