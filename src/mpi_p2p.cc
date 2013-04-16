#include <iostream>
#include <getopt.h>
#include <mpi.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>

//GetOpt Macros
#define GO_NO_ARG  0
#define GO_REQ_ARG 1
#define GO_OPT_ARG 2


int MEG = 1024 * 1024;
int GIG = MEG * 1024;

void peer2peer_datarate (int, int, int, int);

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
        std::cout << "Size of Double = " << sizeof (double) << std::endl;
      }
  }

  peer2peer_datarate (id, size, 11, 10);

  MPI::Finalize ();
  return 0;

}


void
peer2peer_datarate (int id, int p, int groups, int ipg) {
  int i;
  int group, iter;
  int base = 1024 * 1024 / sizeof (double);
  double wtime;
  int group_size;
  int group_data_size;
  MPI::Status status;
  double *data;
  std::vector < double >::iterator it;
  double tsum, tstdev, tmean;
  std::vector < double >iter_time;

  // Run through group of various sizes
  for (group = 0; group < groups; ++group) {
    group_size = base * (int) pow (2, group);
    group_data_size = sizeof (double) * group_size;
    data = new double[group_size];
    iter_time.clear ();

    // Run through a set number of iterations
    for (iter = 0; iter < ipg; ++iter) {
      if (id == 0) {
        // Generate Data
        for (i = 0; i < group_size; ++i)
          data[i] = (float) rand () / (float) RAND_MAX;

        //
        // Send data to peer.
        //
        // First Save start time.
        wtime = MPI::Wtime ();
        // Send data
        MPI::COMM_WORLD.Send (data, group_size, MPI::DOUBLE, 1, 0);
        // Receive data
        MPI::COMM_WORLD.Recv (data, group_size, MPI::DOUBLE, 1, 0,
            status);
        // Record Time Diff
        wtime = MPI::Wtime () - wtime;
        iter_time.push_back (wtime);
      
      } else {
        MPI::COMM_WORLD.Recv (data, group_size, MPI::DOUBLE, 0, 0, status);
        MPI::COMM_WORLD.Send (data, group_size, MPI::DOUBLE, 0, 0);
      }

    }
    
    if (id == 0) {
      // Calculate Mean
      tsum = 0;
      for (it = iter_time.begin (); it != iter_time.end (); ++it)
        {
          tsum += 2 * (double) group_data_size / *it;
        }
      tmean = tsum / (double) iter_time.size ();

      // Calculate STDEV
      tsum = 0;
      for (it = iter_time.begin (); it != iter_time.end (); ++it)
        {
          tsum += pow ((2 * (double) group_data_size / *it - tmean), 2);
        }
      tstdev = sqrt (tsum / (double) iter_time.size ());

      //std::cout << (double)sizeof(double)*group_size/MEG << " " << tmean/MEG << " " << tstdev/MEG << std::endl;
      printf ("%4d %9g %9g\n", group_data_size / MEG, tmean / MEG,
        tstdev / MEG);
    }
  }
}
