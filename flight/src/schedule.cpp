#include <schedule.h>

using namespace std;


pthread_mutex_t schedule_lock;

list<struct Schedule *> schedule;
BoundedBuffer<struct Schedule*> *bb;
Airport *airport;
int max_items; // total number of items in the ledger
int con_items; // total number of items consumed

/**
 * @brief Initializes a banking system with a specified number of
 *      - p producer threads
 *      - c consumer threads
 *      - size of the bounded buffer
 *      - ledger file
 *
 * This function sets up a banking. It then creates and
 * initializes the necessary threads to perform banking operations concurrently.
 * After all threads have completed their tasks, it prints the final state of
 * the bank's accounts.
 *
 * @attention
 * - Initialize the bank with 10 accounts.
 * - If `load_ledger()` fails, exit and free allocated memory.
 * - Be careful how you pass the thread ID to ensure the value does not change.
 * - Don't forget to join all created threads.
 *
 * @param p The number of producer threads.
 * @param c The number of consumer threads.
 * @param size The size of the bounded buffer.
 * @param filename The name of the file containing the ledger data.
 * @return void
 *
 */
void InitAirport(int p, int c, int size, char *filename) {
  airport = new Airport();
  bb = new BoundedBuffer<struct Schedule*>(size);
  airport->print_runway();
  if(load_schedule(filename) != 0){
    delete airport;
    delete bb;  
    exit(0);
  }
  pthread_t p_threads[p];
  pthread_t c_threads[c];
  for (int i = 0; i < p; ++i) {
    pthread_create(&p_threads[i], NULL, producer, NULL);
  }
  int *wids = new int[c];
  for (int i = 0; i < c; ++i) {
    wids[i] = i;
    pthread_create(&c_threads[i], NULL, consumer, &wids[i]);
  }
  for (int i = 0; i < p; ++i) {
    pthread_join(p_threads[i], NULL);
  }
  
  for (int i = 0; i < c; ++i) {
    pthread_join(c_threads[i], NULL);
  }
  airport->print_runway();
  delete wids;
}

/**
 * @brief Loads a ledger from a specified file into the banking system.
 *
 * This function reads transaction data from the given file, where each line
 * represents a ledger entry. The format is as follows:
 *   - Account (int): the account number
 *   - Other (int): for transfers, the other account number; otherwise not used
 *   - Amount (int): the amount to deposit, withdraw, or transfer
 *   - Mode (Enum): 0 for deposit, 1 for withdraw, 2 for transfer
 * The function then creates ledger entries and appends them to the ledger list
 * of the banking system.
 *
 * @attention
 * - If the file cannot be opened, the function returns -1, indicating failure.
 * - The function expects a specific file format as indicated above.
 * - Each line in the file corresponds to a ledger entry.
 * - The ledgerID starts with 0.
 *
 * @param filename The name of the file containing the ledger data.
 * @return 0 on success, -1 on failure to open the file.
 */
int load_schedule(char *filename) {

  ifstream input(filename);
  if(!input){cout << "Couldn't read file\n"; return -1;}
  int flightId, fuelPercent, Time, TimeSpentOnRunway, requestTime, completionTime, mode;
  int count = 0;
  while (input >> flightId >> fuelPercent >> Time >> TimeSpentOnRunway >> requestTime >> completionTime >> mode){
    Schedule* sched = new Schedule();
    sched->flightID = flightId;
    sched->fuelPercent = fuelPercent;
    sched->scheduledTime = Time;
    sched->timeSpentOnRunway = TimeSpentOnRunway;
    sched->requestTime = requestTime;
    sched->completionTime = completionTime;
    sched->mode = mode;
    schedule.push_back(sched);
  }
  max_items = count;
  return 0;
}

/**
 * @brief consumer function for processing ledger entries concurrently.
 *
 * This function represents a consumer thread responsible for processing ledger
 * entries from the bounded buffer. Each thread is assigned a unique ID, and
 * they dequeue ledger entries one by one, performing deposit, withdraw, or
 * transfer operations based on the entry's mode. Threads continue processing
 * until the consumed items = number of ledger items.
 *
 * @attention
 * - The workerID is a unique identifier assigned to each worker thread. Ensure
 * proper dereferencing.
 * - The function uses a mutex (ledger_lock) to ensure thread safety while
 * accessing the global ledger.
 * - It continuously dequeues ledger entries, processes them, and updates the
 * bank's state accordingly.
 * - The worker handles deposit (D), withdraw (W), and transfer (T) operations
 * based on the ledger entry's mode.
 *
 * @param workerID A pointer to the unique identifier of the worker thread.
 * @return NULL after completing ledger processing.
 */
void *consumer(void *workerID) {
  
  while (true){
    Schedule* item;
    pthread_mutex_lock(&schedule_lock);
    if (max_items <= con_items){
      pthread_mutex_unlock(&schedule_lock);
      return NULL;
    }
    con_items++;

    

    pthread_mutex_unlock(&schedule_lock);
    item = bb->remove();
    if (item->mode == 0){
      airport->takeoff(*(int *)workerID, item->flightID, item->fuelPercent, item->scheduledTime, item->timeSpentOnRunway);
    }
    else if (item->mode == 1){
      airport->landing(*(int *)workerID, item->flightID, item->fuelPercent, item->scheduledTime, item->timeSpentOnRunway);
    }
    else{
      return NULL;
    }
  }
}

/**
 * @brief Producer thread function that transfers ledger entries to the bounded buffer.
 *
 * This function acts as the producer. It repeatedly removes ledger
 * entries from a shared ledger container and appends them to a bounded buffer for further processing.
 * The function employs a mutex (schedule_lock) to ensure exclusive access to the shared ledger while
 * checking and modifying its contents.
 *
 * @param[in] unused A pointer to any data (unused in this implementation).
 * @return Always returns NULL.
 *
 * @details
 * - While the ledger is not empty, it:
 *   - Retrieves the first ledger entry.
 *   - Removes the entry from the ledger.
 *   - Appends the entry to the bounded buffer.
 *
 * @note The function should be thread-safe and ensure
 * that the ledger is empty after all entries have been processed.
 */
 void* producer(void *) {
  
  while (true) {
    Schedule* next = nullptr;

    pthread_mutex_lock(&schedule_lock);
    if (!schedule.empty()) {
      next = schedule.front();
      schedule.pop_front();
    } else {
      pthread_mutex_unlock(&schedule_lock);
      return NULL;
    }
    pthread_mutex_unlock(&schedule_lock);

    bb->append(next); 
  }

  return NULL;
}