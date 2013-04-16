#include <iostream>
#include <getopt.h>
#include <mpi.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <utility>

//GetOpt Macros
#define GO_NO_ARG  0
#define GO_REQ_ARG 1
#define GO_OPT_ARG 2


int MEG = 1024 * 1024;
int GIG = MEG * 1024;

void peer2peer_datarate (int, int, int, int);

typedef std::vector<std::pair<int, int>> pair_vector; 

int
main (int argc, char **argv)
{
  bool verbose = false;
  unsigned int MAX_MSG = pow (10, 6);

  MPI::Init (argc, argv);
  int size = MPI::COMM_WORLD.Get_size ();
  int id = MPI::COMM_WORLD.Get_rank ();

  if (size < 2) {
      if (id == 0)
        std::cerr << "Error: This test needs at least two processes." << std::endl;
      MPI::Finalize ();
      return 1;
  }

  // Seed the random number generator
  srand ((unsigned) time (0));

  // Seperate into root and non-root processes
  if (id == 0) {
      // Parse Options
      //
      static struct option long_options[] = {
                                              {"verbose", GO_NO_ARG, 0, 'v'},
                                            };
      int options_index = 0;
      int c;
      while ((c = getopt_long (argc, argv, "v",
			       long_options, &options_index)) != -1) {
	      switch (c) {
	        case 'v':
	          std::cout << "Verbose output set." << std::endl;
            verbose = true;
            break;

          default:
            std::cerr << "Getopt returned unknow caracter code" << c << std::endl;
	      }
	    }

      if (verbose) {
        std::cout << "Starting with " << size << " processes." << std::endl;
      }
  }

  peer2peer_datarate (id, size, 11, 10);

  MPI::Finalize ();
  return 0;

}

void
handshake() {
  int n  = MPI::COMM_WORLD.Get_size();
  int id = MPI::COMM_WORLD.Get_rank();

}

pair_vector genPairs(int n) {
  int i,j;
  pair_vector pairs;

  for ( i = 0 ; i < n ; ++i ) {
    for ( j = i+1 ; j < n ; ++j) {
      pairs.push_back(std::make_pair<int, int>);
    }
  }
  return pairs;
}
