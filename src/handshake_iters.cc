#include <iostream>
#include <getopt.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <utility>
#include <unordered_map>
#include <queue>

//GetOpt Macros
#define GO_NO_ARG  0
#define GO_REQ_ARG 1
#define GO_OPT_ARG 2


typedef std::vector<std::pair<int, int>> pair_vector; 

pair_vector genPairs(int);

int
main (int argc, char **argv)
{
  int n = 1400;
  pair_vector pairs = genPairs(n);
  std::pair<int, int> pair;

  std::unordered_map<int, int> seen;
  std::vector<pair_vector::iterator> toremove;
  std::vector<pair_vector::iterator>::iterator trm_it;

  pair = pairs[0];
  
  int iter = 0;
  int iter_size = 0;
  int inel = 0;

  std::cout << "#It #Parallelism" << std::endl;

  // Run through all pairs, maximizing the number of pairs per turn.
  while ( pairs.size() > 0 ) {
    ++iter;
    seen.clear();
    toremove.clear();
    iter_size = 0;
    inel = 0;

    // Build the pairs queue
    for ( pair_vector::iterator it = pairs.begin() ; it != pairs.end() ; ++it) {
      pair = *it;

      std::unordered_map<int, int>::const_iterator got_na = seen.find(pair.first);
      std::unordered_map<int, int>::const_iterator got_nb = seen.find(pair.second);
      
      // If we've already queued a node this turn, skip it.
      if ( got_na != seen.end() or got_nb != seen.end() ) {
        inel++;
        continue;
      }
      // Add nodes to queue and remove it from the pairs
      toremove.push_back(it);
      seen[pair.first]  = pair.second;
      seen[pair.second] = pair.first;
      iter_size++;
    }

    std::cout << iter << " " << iter_size << " " << inel << std::endl;
    for ( trm_it = --toremove.end(); trm_it != --toremove.begin() ; --trm_it ) {
      pair = **trm_it;
      pairs.erase(*trm_it);
    }
  }

  return 0;
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
