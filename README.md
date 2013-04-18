MPI_Benchmarks
==============

A set of benchmarks to test MPI over IB and Ethernet.

MPI_P2P
-------

The peer to peer bandwidth test will give you statistics on a single node to another node.
To run this execute 
```mpiexec -npernode 1 -H node1,node2 ./mpi_p2p -v```

MPI_HANDSHAKE
-------------

This test will preform a bandwitdth test between every pair of nodes in the hostlist provided.
Still under development.
