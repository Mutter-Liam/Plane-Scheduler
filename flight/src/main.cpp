#include <schedule.h>

int main(int argc, char* argv[]) {

  if (argc != 6) {
    cerr << "Usage: " << argv[0] << " <num_producers> <num_consumers> <bb_size> <leader_file> <scheduling_alg_type>\n" << endl;
    exit(-1);
  }

  int p = atoi(argv[1]);       // number of producer threads
  int c = atoi(argv[2]);       // number of consumer threads
  int size = atoi(argv[3]);   // size of the bounded buffer
  int algType = atoi(argv[5]);
  InitAirport(p, c, size, argv[4], algType);

  return 0;
}
