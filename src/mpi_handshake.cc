#include <iostream>
#include <getopt.h>
#include <mpi.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <utility>
#include <unordered_map>
#include <queue>
#include <stack>

//GetOpt Macros
#define GO_NO_ARG  0
#define GO_REQ_ARG 1
#define GO_OPT_ARG 2


int MEG = 1024 * 1024;
int GIG = MEG * 1024;

void peer2peer_datarate (int, int, int, int);

typedef std::vector<std::pair<int, int>> pair_vector; 

pair_vector genPairs(int);
void handshake(void);

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
  

  //
  // Run the handshakes
  //
  handshake();


  MPI::Finalize ();
  return 0;

}

/*******************************************************************************
 * handshake will run through all possible pairs of nodes and print the
 * time between them. 
*******************************************************************************/

void
handshake(void) {
  
  int i;

  int n  = MPI::COMM_WORLD.Get_size();
  int id = MPI::COMM_WORLD.Get_rank();
  
  char hostname[MPI_MAX_PROCESSOR_NAME];
  int hostname_len = 0;
  MPI::Get_processor_name(hostname, hostname_len);
  MPI::Status mstatus;

  int keep_going = 0;

  std::vector<double> wtime;
  double wtime_std;
  double wtime_mean;
  double * data;

  int msg_length = 10;
  int msg_iters  = 10;

  std::unordered_map<int, std::string> id2name;

  /*****************
   * Get Hostnames *
   *****************/
  char rhostname[MPI_MAX_PROCESSOR_NAME];

  int partner;

  if ( id == 0 ) {
    // Set the root node host name
    id2name[0] = std::string(hostname, hostname_len);
    std::cout << "Root node: " << id2name[0] << std::endl;

    // Gather all other hostnames
    for ( i = 1 ; i < n ; ++i  ) {
      MPI::COMM_WORLD.Recv(rhostname, MPI_MAX_PROCESSOR_NAME, MPI::CHAR, i, 0 , mstatus);
      id2name[i] = std::string(rhostname, mstatus.Get_count(MPI::CHAR));
      std::cout << "Found host: " << i << " : " << id2name[i] << std::endl;
    }
  } else {
    // Send the root node this nodes's hostname
    MPI::COMM_WORLD.Send(hostname, hostname_len, MPI::CHAR, 0, 0);
  }
   
  /****************
   * Do Hanshakes *
   ****************/
  if ( id == 0 ) {
    /* Root Node */
    pair_vector pairs = genPairs(n);
    std::pair<int, int> pair;

    std::unordered_map<int, int> seen;
    std::stack<pair_vector::iterator> toremove;

    partner = 0;
    pair = pairs[0];
    
    int iter = 0;
    int iter_size = 0;

    // Run through all pairs, maximizing the number of pairs per turn.
    while ( pairs.size() > 0 ) {
      ++iter;

      //
      // Generate List of Pairs first
      //

      // Clear seen and removal list
      seen.clear();
      iter_size = 0;

      // Build the pairs queue
      for ( pair_vector::iterator it = pairs.begin() ; it != pairs.end() ; ++it) {
        pair = *it;

        std::unordered_map<int, int>::const_iterator got_na = seen.find(pair.first);
        std::unordered_map<int, int>::const_iterator got_nb = seen.find(pair.second);
        
        // If we've already queued a node this turn, skip it.
        if ( got_na != seen.end() or got_nb != seen.end() ) {
          continue;
        }
        // Add nodes to queue and remove it from the pairs
        toremove.push(it);
        seen[pair.first]  = pair.second;
        seen[pair.second] = pair.first;
        iter_size++;
      }

      //
      // Dispatch orders
      //
      while ( ! toremove.empty() ) {
        pair = *toremove.top();
        pairs.erase(toremove.top());
        toremove.pop();
        
        if ( pair.first == 0 ) {
          partner = pair.second;
        } else {
          // Send keep_going = True signal
          MPI::COMM_WORLD.Send(&keep_going, 1, MPI::INT, pair.first,  0);
          MPI::COMM_WORLD.Send(&keep_going, 1, MPI::INT, pair.second, 0);

          // Send Partners to pairs
          MPI::COMM_WORLD.Send(&pair.first,  1, MPI::INT, pair.second, 0);
          MPI::COMM_WORLD.Send(&pair.second, 1, MPI::INT, pair.first,  0);
        }
        
      }
    }
  } else {
    // Cycle for All (non-root) nodes
    while (true) {
      // break if the root process is out of pairs
      MPI::COMM_WORLD.Recv(&keep_going, 1, MPI::INT, 0, 0, mstatus);
      if ( !keep_going )
        break;

      // Recieve parter to communicate with.
      MPI::COMM_WORLD.Recv(&partner, 1, MPI::INT, 0, 0, mstatus);

      // Let everything sync
      MPI::comm::Barrier();
    
      // Now if this id is lower than the partnet id, this process send first and tracks the time.
      if ( partner > id ) {
        data = new double[msg_length];
        // Create random data
        for( int i = 0 ; i < msg_length ; ++i ) {
          data[i] = (float) rand()/(float) RAND_MAX;
        }

        for ( int msg_iter = 0 ; msg_iter < msg_iters ; ++msg_iter ) {
          // Save start time
          wtime.push_back(-1.0*MPI::Wtime());

          // Send then recieve
          MPI::COMM_WORLD.Send(data, msg_length, MPI::DOUBLE, partner, 0);
          MPI::COMM_WORLD.Recv(data, msg_length, MPI::DOUBLE, partner, 0, mstatus);
          
          // Calculate RTT/2
          wtime.back() += MPI::Wtime();
          wtime.back() /= 2;
        }
        // Calculate mean and std
        for ( std::vector<double>::iterator it = wtime.begin() ; it != wtime.end() ; ++it ) {
          wtime_mean += *it/msg_length;
          wtime_mean /= wtime.size();
        }
        for ( std::vector<double>::iterator it = wtime.begin() ; it != wtime.end() ; ++it ) {
          wtime_std += pow( (*it - wtime_mean), 2); 
        }
        wtime_std = sqrt( wtime_std / wtime.size() );

        // Send Results to root
        double results[2];
        results[0] = wtime_mean;
        results[1] = wtime_std;
        MPI::COMM_WORLD.Send(results, 2, MPI::DOUBLE, 0, 0);
      } else {
        for ( int msg_iter = 0 ; msg_iter < msg_iters ; ++msg_iter ) {
          // Recieve then Send 
          MPI::COMM_WORLD.Recv(data, msg_length, MPI::DOUBLE, partner, 0, mstatus);
          MPI::COMM_WORLD.Send(data, msg_length, MPI::DOUBLE, partner, 0);
        }
      }
      

    }
  }

}

pair_vector genPairs(int n) {
  int i,j;
  pair_vector pairs;

  pairs.reserve((n*(n+1))>>1);

  for ( i = 0 ; i < n ; ++i ) {
    for ( j = i+1 ; j < n ; ++j) {
      pairs.push_back(std::make_pair(i, j));
    }
  }
  return pairs;
}
