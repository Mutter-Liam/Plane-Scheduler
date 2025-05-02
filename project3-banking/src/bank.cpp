#include <bank.h>

/**
 * @brief prints account information
 */
void Bank::print_account() {
  for (int i = 0; i < num; i++) {
    pthread_mutex_lock(&accounts[i].lock);
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance
         << endl;
    pthread_mutex_unlock(&accounts[i].lock);
  }

  pthread_mutex_lock(&bank_lock);
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Bank::recordFail(string message) {
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_fail++;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
 */
void Bank::recordSucc(string message) {
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_succ++;
  pthread_mutex_unlock(&bank_lock);
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
Bank::Bank(int N) {
  pthread_mutex_init(&bank_lock, NULL);
  accounts = new Account[N];
  for (int i = 0; i < N; i++) {
    accounts[i].accountID = i;
    accounts[i].balance = 0;
    pthread_mutex_init(&accounts[i].lock, NULL);
  }

  num = N;
  num_fail = 0;
  num_succ = 0;
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
Bank::~Bank() {
  pthread_mutex_destroy(&bank_lock);
  for (int i = 0; i < num; i++){
    Account* acc = &accounts[i];
    pthread_mutex_t* l = &acc->lock;
    pthread_mutex_destroy(l);
  }
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
int Bank::deposit(int workerID, int ledgerID, int accountID, int amount) {

  Account* deposit_account = &accounts[accountID];
  pthread_mutex_t* account_lock = &deposit_account->lock;

  pthread_mutex_lock(account_lock);
  if (deposit_account->balance + amount < 0){
    recordFail(DEPOSIT_MSG(SUCC, workerID, ledgerID, accountID, amount));
    pthread_mutex_unlock(account_lock);
    return -1;
  }
  deposit_account->balance += amount;
  pthread_mutex_unlock(account_lock);
  string log = DEPOSIT_MSG(SUCC, workerID, ledgerID, accountID, amount);
  recordSucc(log);

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
int Bank::withdraw(int workerID, int ledgerID, int accountID, int amount) {

  Account* withdraw_account = &accounts[accountID];
  pthread_mutex_t* account_lock = &withdraw_account->lock;
  long balance;
  string log;
  bool failure;

  pthread_mutex_lock(account_lock);
  balance = withdraw_account->balance;
  if (balance - amount >= 0){
    withdraw_account->balance -= amount;
    failure = false;
  }
  else{
    failure = true;
  }
  pthread_mutex_unlock(account_lock);
  if (!failure){
    log = WITHDRAW_MSG(SUCC, workerID, ledgerID, accountID, amount);
    recordSucc(log);

    return 0;
  }
  else{
    log = WITHDRAW_MSG(ERR, workerID, ledgerID, accountID, amount);
    recordFail(log);

    return -1;
  }
}

/**
 * @brief Transfer funds from one account to another.
 *
 * @details
 * This function transfers the specified amount from the source account to the
 * destination account. It ensures that there is enough money in the source
 * account for the transfer and carefully handles the locking order to prevent
 * deadlock.
 *
 * @attention
 * - The function requires careful consideration of the locking order to prevent
 *     deadlock.
 * - It ensures that there is enough money in the source account before
 *     performing the transfer.
 * - On success it logs: `[ SUCCESS ] TID: {workerID}, LID: {ledgerID}, Acc:
 *      {accountID} TRANSFER ${amount} TO Acc: {destID}`
 * - On faiure it logs: `[ ERROR ] TID: {workerID}, LID: {ledgerID}, Acc:
 *      {accountID} TRANSFER ${amount} TO Acc: {destID}`
 * - Transfer from srcID = n to destID = n is a failure
 *
 * @param workerID The ID of the worker (thread).
 * @param ledgerID The ID of the ledger entry.
 * @param srcID The account ID to transfer money from.
 * @param destID The account ID to receive the money.
 * @param amount The amount to transfer.
 * @return 0 on success, -1 on error.
 */
int Bank::transfer(int workerID, int ledgerID, int srcID, int destID,
                   unsigned int amount) {

  if (srcID == destID){ 
    recordFail(TRANSFER_MSG(ERR, workerID, ledgerID, srcID, destID, amount));
    return -1;
  }
  Account* withdraw_account = &accounts[srcID];
  Account* deposit_account = &accounts[destID];
  bool failed;
  pthread_mutex_t* withdraw_lock = &withdraw_account->lock;
  pthread_mutex_t* deposit_lock = &deposit_account->lock;
  if (srcID < destID){
    pthread_mutex_lock(withdraw_lock);
    pthread_mutex_lock(deposit_lock);
  }
  else{
    pthread_mutex_lock(deposit_lock);  
    pthread_mutex_lock(withdraw_lock);
  }
  long withdraw_balance = withdraw_account->balance;
  if(withdraw_balance - amount >= 0){
    withdraw_account->balance -= amount;
    deposit_account->balance += amount;
    recordSucc(TRANSFER_MSG(SUCC, workerID, ledgerID, srcID, destID, amount));
    failed = false;
  }
  else{
    recordFail(TRANSFER_MSG(ERR, workerID, ledgerID, srcID, destID, amount));
    failed = true;
  }
  if (srcID < destID) {
    pthread_mutex_unlock(deposit_lock);
    pthread_mutex_unlock(withdraw_lock);
  } else {
    pthread_mutex_unlock(withdraw_lock);
    pthread_mutex_unlock(deposit_lock);
  }

  return (failed) ? -1 : 0;
}