#include <airport.h>
#include <boundedBuffer.h>
/**
 * @brief Prints the status of all airport runways.
 * 
 * Iterates through all runways and displays their respective takeoff and landing counts.
 * Ensures thread safety using mutex locks while accessing shared data.
 * Also prints the total number of airport-wide takeoffs and landings.
 */
void Airport::print_runway() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_lock(&runways[i].lock);
    cout << "ID# " << runways[i].runwayID << " | " << "takeoffs: " << runways[i].takeoffs << " landings: " << runways[i].landings<< endl;
    pthread_mutex_unlock(&runways[i].lock);
  }

  pthread_mutex_lock(&airport_lock);
  cout << "Airport takeoffs: " << num_takeoffs << " Airport landings: " << num_landings << endl;
  cout << "Average Response Time: " << this->getRespTime() << endl;
  cout << "Average Fuel Burning: " << this->getFuelBurn() << endl;
  pthread_mutex_unlock(&airport_lock);
}

/**
 * @brief Records a landing event for a specific runway.
 * 
 * Increments the landing count for the given runway and updates the total 
 * number of airport-wide landings. Logs the provided message to the console.
 * 
 * @param message The log message to be displayed.
 * @param runwayID The ID of the runway where the landing occurred.
 */
void Airport::recordLanding(string message, int runwayID) {
  runways[runwayID].landings++;
  num_landings++;
  cout << message << endl;
}

/**
 * @brief Records a takeoff event for a specific runway.
 * 
 * Increments the takeoff count for the given runway and updates the total 
 * number of airport-wide takeoffs. Logs the provided message to the console.
 * 
 * @param message The log message to be displayed.
 * @param runwayID The ID of the runway where the takeoff occurred.
 */
void Airport::recordTakeoff(string message, int runwayID) {
  runways[runwayID].takeoffs++;
  num_takeoffs++;
  cout << message << endl;
}

/***************************************************
 * DO NOT MODIFY ABOVE CODE
 ****************************************************/

/**
 * @brief Construct a new Airport object.
 *
 * @details
 * This constructor initializes the private variables of the Airport class, 
 * creates an array of runway objects, and initializes each runway with default 
 * values. Each runway is assigned a unique ID and has its takeoff and landing 
 * counts set to zero. Additionally, mutexes and condition variables are 
 * initialized to ensure thread safety during concurrent operations.
 *
 * @param N The number of runways to be tracked in the airport.
 */
Airport::Airport(int N)
    : available_runways(2)
{
    pthread_mutex_init(&airport_lock, NULL);
    pthread_cond_init(&runway_available_cond, NULL);
    runways = new Runway[2];
    for (int i = 0; i < 2; i++) {
        runways[i].runwayID = i;
        runways[i].takeoffs = 0;
        runways[i].landings = 0;
        runways[i].time = 0;
        pthread_mutex_init(&runways[i].lock, NULL);
    }

    num = N;
    num_takeoffs = 0;
    num_landings = 0;
    respTimeSum = 0;
    fuelBurnSum = 0;
}


/**
 * @brief Destroy the Airport object.
 *
 * @details
 * This destructor is responsible for cleaning up the resources used by the 
 * Airport object. It ensures that all mutexes associated with runways and the 
 * airport-wide operations are properly destroyed. Additionally, it releases 
 * allocated memory for the runway array.
 *
 * @attention
 * - This destructor is automatically called when an Airport object goes out of 
 * scope or is explicitly deleted.
 */

 Airport::~Airport() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_destroy(&runways[i].lock);
  }
  pthread_mutex_destroy(&airport_lock);
  pthread_cond_destroy(&runway_available_cond);
  delete[] runways;
}

/**
 * @brief Handles a flight takeoff process.
 *
 * @details
 * This function attempts to acquire an available runway for a flight takeoff. 
 * It ensures thread safety by using mutex locks to manage runway access and 
 * condition variables to wait for availability when necessary. Once a runway 
 * is acquired, the takeoff is recorded using `recordTakeoff()`, and the runway 
 * is released for other flights to use.
 *
 * @param workerID The ID of the worker (thread) handling the takeoff.
 * @param flightID The ID of the flight taking off.
 * @param fuelPercentage The remaining fuel percentage of the flight.
 * @param scheduledTime The scheduled departure time of the flight.
 * @param timeSpentOnRunway The actual time the flight spent on the runway.
 * @param actualTime The actual time at which the takeoff occurred.
 * @param completionTime The time when the takeoff process was completed.
 * @return 0 on success.
 */

int Airport::takeoff(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway, int actualTime, int completionTime) {

  Runway *chosen_runway = nullptr;
  int runwayID = -1;
  pthread_mutex_lock(&airport_lock);
  while (true) {
    for (int i = 0; i < num; i++) {
      if (pthread_mutex_trylock(&runways[i].lock) == 0) { //wait till runway available
        chosen_runway = &runways[i];
        runwayID = i;
        break;
      } 
    }
    if (chosen_runway) {
      respTimeSum += (actualTime - scheduledTime);
      fuelBurnSum += (fuelPercentage - (actualTime - scheduledTime));
      break;
    }
    pthread_cond_wait(&runway_available_cond, &airport_lock);//signal wait for runway
  }
  pthread_mutex_unlock(&airport_lock);
  recordTakeoff(TAKEOFF_MSG(workerID, flightID, scheduledTime, runwayID, fuelPercentage, actualTime, completionTime), runwayID);

  //unlock runway and signal waiting flights
  pthread_mutex_unlock(&chosen_runway->lock);
  pthread_mutex_lock(&airport_lock);
  pthread_cond_signal(&runway_available_cond);
  pthread_mutex_unlock(&airport_lock);

  return 0;
}

/**
 * @brief Handles a flight landing process.
 *
 * @details
 * This function attempts to acquire an available runway for a flight to land. 
 * It ensures thread safety by using mutex locks to manage runway access and 
 * condition variables to wait for availability when necessary. Once a runway 
 * is secured, the landing is recorded using `recordLanding()`, and the runway 
 * is released for other flights to use.
 *
 * @param workerID The ID of the worker (thread) handling the landing.
 * @param flightID The ID of the flight landing.
 * @param fuelPercentage The remaining fuel percentage of the flight.
 * @param scheduledTime The scheduled arrival time of the flight.
 * @param timeSpentOnRunway The actual time the flight spent on the runway.
 * @param actualTime The actual time at which the landing occurred.
 * @param completionTime The time when the landing process was completed.
 * @return 0 on success.
 */
int Airport::landing(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway, int actualTime, int completionTime) {

  Runway *chosen_runway = nullptr;
  int runwayID = -1;
  pthread_mutex_lock(&airport_lock);
  while (true) {
    for (int i = 0; i < num; i++) {
      if (pthread_mutex_trylock(&runways[i].lock) == 0) { //wait till runway available
        chosen_runway = &runways[i];
        runwayID = i;
        break;
      } 
    }
    if (chosen_runway) {
      break;
    }
    pthread_cond_wait(&runway_available_cond, &airport_lock);//signal wait for runway
  }
  pthread_mutex_unlock(&airport_lock);
  recordLanding(LANDING_MSG(workerID, flightID, scheduledTime, runwayID, fuelPercentage, actualTime, completionTime), runwayID);

  //unlock runway and signal waiting flights
  pthread_mutex_unlock(&chosen_runway->lock);
  pthread_mutex_lock(&airport_lock);
  pthread_cond_signal(&runway_available_cond);
  pthread_mutex_unlock(&airport_lock);

  return 0;
}
