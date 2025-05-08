#include <airport.h>
#include <boundedBuffer.h>
/**
 * @brief prints account information
 */
void Airport::print_runway() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_lock(&runways[i].lock);
    cout << "ID# " << runways[i].runwayID << " | " << "takeoffs: " << runways[i].takeoffs << " landings: " << runways[i].landings<< endl;
    pthread_mutex_unlock(&runways[i].lock);
  }

  pthread_mutex_lock(&airport_lock);
  cout << "Airport takeoffs: " << num_takeoffs << " Airport landings: " << num_landings << endl;
  pthread_mutex_unlock(&airport_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Airport::recordLanding(string message, int runwayID) {
  runways[runwayID].landings++;
  num_landings++;
  cout << message << endl;
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
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
 * @brief Construct a new Bank object.
 *
 * @details
 * This constructor initializes the private variables of the Bank class, creates
 * a new array of type Accounts with a specified size (N), and initializes each
 * account. The accounts are identified by their accountID, and their initial
 * balance is set to 0. Additionally, mutexes are initialized for each account
 * to ensure thread safety during concurrent operations.
 *
 *
 * @param N The number of accounts to be created in the bank.
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
}


/**
 * @brief Destroy the Bank object.
 *
 * @details
 * This destructor is responsible for cleaning up the resources used by the Bank
 * object. It ensures that all locks associated with the bank and its accounts
 * are destroyed, and the allocated memory for accounts is freed. Additionally,
 * the bank-wide mutex is destroyed.
 *
 * @attention
 * - This destructor is automatically called when a Bank object goes out of
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
 * @brief Adds money to an account.
 *
 * @details
 * This function deposits the specified amount into the specified account and
 * logs the transaction in the following format:
 *   `[ SUCCESS ] TID: {workerID}, LID: {ledgerID}, Acc: {accountID} DEPOSIT ${amount}`
 * using the DEPOSIT_MSG() macro for consistent formatting.
 *
 * @param workerID The ID of the worker (thread).
 * @param ledgerID The ID of the ledger entry.
 * @param accountID The account ID to deposit.
 * @param amount The amount to deposit.
 * @return 0 on success.
 */
int Airport::takeoff(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway) {

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
  recordTakeoff(TAKEOFF_MSG(workerID, flightID, scheduledTime, runwayID, fuelPercentage), runwayID);

  //unlock runway and signal waiting flights
  pthread_mutex_unlock(&chosen_runway->lock);
  pthread_mutex_lock(&airport_lock);
  pthread_cond_signal(&runway_available_cond);
  pthread_mutex_unlock(&airport_lock);

  return 0;
}

/**
 * @brief Withdraws money from an account.
 *
 * @details
 * This function attempts to withdraw the specified amount from the specified
 * account. It checks two cases:
 *   - Case 1: If the withdrawal amount is less than or equal to the account
 * balance, the withdrawal is successful, and the transaction is logged as
 * `[ SUCCESS ] TID: {workerID}, LID: {ledgerID}, Acc: {accountID} WITHDRAW ${amount}`.
 *   - Case 2: If the withdrawal amount exceeds the account balance, the
 * withdrawal fails, and the transaction is logged as
 * `[ ERROR ] TID: {workerID}, LID: {ledgerID}, Acc: {accountID} WITHDRAW ${amount}`.
 *
 * @attention
 * - The function ensures that the account has a large enough balance for a
 * successful withdrawal.
 *
 * @param workerID The ID of the worker (thread).
 * @param ledgerID The ID of the ledger entry.
 * @param accountID The account ID to withdraw from.
 * @param amount The amount to withdraw.
 * @return 0 on success, -1 on failure.
 */
int Airport::landing(int workerID, int flightID, int fuelPercentage, int scheduledTime, int timeSpentOnRunway) {

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
  recordLanding(LANDING_MSG(workerID, flightID, scheduledTime, runwayID, fuelPercentage), runwayID);

  //unlock runway and signal waiting flights
  pthread_mutex_unlock(&chosen_runway->lock);
  pthread_mutex_lock(&airport_lock);
  pthread_cond_signal(&runway_available_cond);
  pthread_mutex_unlock(&airport_lock);

  return 0;
}
