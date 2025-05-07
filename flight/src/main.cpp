#include <schedule.h>

int main(int argc, char* argv[]) {

  if (argc != 5) {
    cerr << "Usage: " << argv[0] << " <num_producers> <num_consumers> <bb_size> <leader_file>\n" << endl;
    exit(-1);
  }

  int p = atoi(argv[1]);       // number of producer threads
  int c = atoi(argv[2]);       // number of consumer threads
  int size = atoi(argv[3]);   // size of the bounded buffer
  InitAirport(p, c, size, argv[4]);

  return 0;
}
