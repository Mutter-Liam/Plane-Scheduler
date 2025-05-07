#include <boundedBuffer.h>

/**
 * DO NOT DELETE
 */
template class BoundedBuffer<Flight*>;
template class BoundedBuffer<Runway*>;
template class BoundedBuffer<Schedule*>;
template class BoundedBuffer<int>;

/**
 * @brief Constructs a bounded buffer with a fixed capacity.
 *
 * This constructor creates an instance of a BoundedBuffer, allocating the internal storage for
 * up to N elements of type T. It initializes the state variables and the thread synchronization
 * required to safely manage concurrent access to the buffer.
 *
 * @tparam T The type of elements stored in the buffer.
 * @param N The maximum number of elements that the buffer can hold.
 *
 * @details
 * - Allocates memory for the buffer
 * - Initializes the following internal state:
 *   - `buffer_size`: Set to N, representing the capacity of the buffer.
 *   - `buffer_cnt`: Initialized to 0, tracking the current number of elements in the buffer.
 *   - `buffer_first` and `buffer_last`: Both initialized to 0, used as indices to manage the
 *     circular buffer logic.
 * - Sets up thread synchronization
 *
 * @note
 * - Ensure that the corresponding destructor properly frees the allocated memory and destroys
 *   the synchronization primitives to avoid resource leaks.
 *
 * Example usage:
 * @code
 * BoundedBuffer<int> buffer(100);  // Creates a bounded buffer for 100 integers.
 * @endcode
 */
template <typename T>
BoundedBuffer<T>::BoundedBuffer(int N) {
  // TODO: constructor to initiliaze all the varibales declared in
  buffer = new T[N];//allocate memory for buffer
  
  //set up internal state
  buffer_size = N;
  buffer_cnt = 0;
  buffer_first = 0;
  buffer_last = 0;

  //set up thread sync things
  pthread_mutex_init(&buffer_lock, NULL);
  pthread_cond_init(&buffer_not_full, NULL);
  pthread_cond_init(&buffer_not_empty, NULL);
}

/**
 * @brief Destroys the bounded buffer and releases its resources.
 *
 * This destructor cleans up the resources allocated for the BoundedBuffer instance.
 * It frees the dynamically allocated memory for the internal buffer and destroys the
 * associated POSIX thread synchronization primitives (mutex and condition variables).
 *
 * @tparam T The type of elements stored in the buffer.
 */
template <typename T>
BoundedBuffer<T>::~BoundedBuffer() {
  // TODO: destructor to clean up anything necessary
  pthread_mutex_destroy(&buffer_lock);
  pthread_cond_destroy(&buffer_not_full);
  pthread_cond_destroy(&buffer_not_empty);

  delete[] buffer;
}

/**
 * @brief Appends an item to the circular buffer.
 *
 * This method adds an element to the bounded buffer in a thread-safe manner.
 * It locks the internal mutex, waits for available space if the buffer is full,
 * and then adds the element into the buffer. Once the element is added, the
 * method signals any waiting threads that the buffer is not empty and releases the lock.
 *
 * @tparam T The type of elements stored in the buffer.
 * @param data The element to be appended to the buffer.
 *
 * * @note This function will block if the buffer is full until an element is removed by another thread.
 *
 * Example usage:
 * @code
 * BoundedBuffer<int> buffer(10);
 * buffer.append(42);  // Appends the value 42 to the buffer.
 * @endcode
 */
template <typename T>
void BoundedBuffer<T>::append(T data) {
  // TODO: append a data item to the circular buffer
  pthread_mutex_lock(&buffer_lock);
  while (buffer_cnt == buffer_size) {//make sure buffer not full
    pthread_cond_wait(&buffer_not_full, &buffer_lock);
  }
  buffer[buffer_last] = data;//add new data
  buffer_last = (buffer_last+1) % buffer_size;//circular
  buffer_cnt++;
  pthread_cond_signal(&buffer_not_empty);//added something new -> signal buffer not empty
  pthread_mutex_unlock(&buffer_lock);
}

/**
 * @brief Removes and returns an item from the circular buffer in a thread-safe manner.
 *
 * This method retrieves and removes the top element from the bounded buffer. It ensures thread safety
 * by acquiring a mutex lock and waiting on a condition variable if the buffer is empty. Once an item is available,
 * it updates the internal state of the circular buffer and signals threads waiting for space.
 *
 * @tparam T The type of elements stored in the buffer.
 * @return T The data item removed from the buffer.

 *
 * @note This function will block if the buffer is empty until an element is appended by another thread.
 */
template <typename T>
T BoundedBuffer<T>::remove() {
  // TODO: remove and return a data item from the circular buffer
  pthread_mutex_lock(&buffer_lock);
  while (buffer_cnt == 0) {//make sure buffer not empty
    pthread_cond_wait(&buffer_not_empty, &buffer_lock);
  }
  T removed = buffer[buffer_first];//remove from the front of the buffer
  buffer_first = (buffer_first+1) % buffer_size;//circular;
  buffer_cnt--;
  pthread_cond_signal(&buffer_not_full);//removed something -> signal buffer not full
  pthread_mutex_unlock(&buffer_lock);
  return removed;
}

/**
 * @brief Checks if the bounded buffer is empty.
 *
 * @tparam T The type of elements stored in the buffer.
 * @return bool `true` if the buffer is empty, `false` if it contains one or more items.
 *
 */
template <typename T>
bool BoundedBuffer<T>::isEmpty() {
  // TODO: check is the buffer is empty
  pthread_mutex_lock(&buffer_lock);
  bool b = (buffer_cnt == 0);
  pthread_mutex_unlock(&buffer_lock);
  return b;
}
