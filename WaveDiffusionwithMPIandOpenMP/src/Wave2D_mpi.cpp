
    #include <iostream>
    #include "Timer.h"
    #include <stdlib.h>   // atoi
    #include <math.h>       // pow
    #include <omp.h>    // omp
    #include <mpi.h>    // MPI

    int default_size = 100;  // the default system size
    int defaultCellWidth = 8;
    double c = 1.0;      // wave speed
    double dt = 0.1;     // time quantum
    double dd = 2.0;     // change in system
    //int matrix_size = 100;

    using namespace std;

    int main( int argc, char *argv[] ) {
      // verify arguments
      if ( argc != 5 ) {
        cerr << "usage: Wave2D size max_time interval" << endl;
        return -1;
      }
      int size = atoi( argv[1] );   // the edge length of a 2D simulated square
      int max_time = atoi( argv[2] );   // # steps to simulate wave dissmination(>=2)
      int interval  = atoi( argv[3] );  // #simulation steps needed each time the current simulation sapce is printed out
      int nThreads = atoi(argv[4]);   // #Threads: 1-3

      if ( size < 100 || max_time < 3 || interval < 0 ) {
        cerr << "usage: Wave2D size max_time interval" << endl;
        cerr << "       where size >= 100 && time >= 3 && interval >= 0" << endl;
        return -1;
      }
        
      int my_rank = 0;
      int mpi_size = 1;   // 1 - 4 ranks
        
      // start MPI
      MPI_Init(&argc, &argv);
      MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
            
      // set # threads
      omp_set_num_threads( nThreads );   // 1-3 threads for cssmpi mechine
     
      // create a simulation space
      double z[3][size][size];
      for ( int p = 0; p < 3; p++ )
        for ( int i = 0; i < size; i++ )
          for ( int j = 0; j < size; j++ )
        z[p][i][j] = 0.0; // no wave
        
      // start a timer
      Timer time;
        if (my_rank == 0) {
            time.start( );
        }

     int weight = size / default_size;
     MPI_Status status;
        
     double p = pow(c, 2) * pow((double)(dt/dd), 2); //
     int stripe = size / mpi_size;   // partitioned stripe
     
     // printf("rank[%d]'s range from %d ~ %d\n", my_rank, my_rank * stripe, (my_rank + 1)*stripe - 1 );
      cerr << "rank[" << my_rank << "]'s range from " << my_rank * stripe << " ~ " << (my_rank + 1)*stripe - 1  << endl;
        
        // time = 0;
        // initialize the simulation space: calculate z[0][][]
    // #pragma omp parallel for private(i,j)
      for( int i = my_rank * stripe; i < (my_rank + 1) * stripe; i++ ) {
        for( int j = 0; j < size; j++ ) {
          if( i > 40 * weight && i < 60 * weight  && j > 40 * weight && j < 60 * weight ) {
             z[0][i][j] = 20.0;
          } else {
              z[0][i][j] = 0.0;
          }
        }
      }
        
        // simulate wave diffusion from time = 2
        int t_cur = 1, t_1 = 0, t_2 = -1;
        for ( int t = 1; t < max_time; t++ ) {
            // shift down / rotate z[2][][], z[1][][], and z[0][][].
            t_cur = t % 3;  // t_cur = (t_cur + 1) % 3;
            t_1 = (t - 1) % 3;    // t_1 = (t_1 + 1) % 3;
            t_2 = (t - 2) % 3;    // t_2 = (t_2 + 1) % 3;
            
            // send the bandarary data - start after t == 1
            if (my_rank % 2 == 0) {
                if ((my_rank - 1) >= 0) {
                     MPI_Recv(&(z[t_1][my_rank * stripe - 1][0]),size, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD, &status);
                     MPI_Send(&(z[t_1][my_rank * stripe][0]),size, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD);
                }
                if ((my_rank + 1) < mpi_size) {
                     MPI_Recv(&(z[t_1][(my_rank + 1) * stripe][0]),size, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD, &status);
                     MPI_Send(&(z[t_1][(my_rank + 1) * stripe - 1][0]), size, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD);
                }
            } else {
                if ((my_rank - 1) >= 0) {
                     MPI_Send(&(z[t_1][my_rank * stripe][0]), size, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD);
                     MPI_Recv(&(z[t_1][my_rank * stripe - 1][0]), size, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD, &status);
                }
                if ((my_rank + 1) < mpi_size) {
                     MPI_Send(&(z[t_1][(my_rank + 1) * stripe - 1][0]), size, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD);
                     MPI_Recv(&(z[t_1][(my_rank + 1) * stripe][0]), size, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD, &status);
                }
            }
                
            if (t == 1) {
                #pragma omp parallel for
                for (int i = my_rank * stripe; i < (my_rank + 1) * stripe; i++) {
                    if (i == 0 || i == size - 1) continue;
                    for (int j = 1; j < size - 1; j++) {
                        z[1][i][j] = z[0][i][j] + (p / 2.0) * (z[0][i + 1][j] + z[0][i - 1][j] + z[0][i][j + 1] + z[0][i][j - 1] - 4.0 * z[0][i][j]);
                    }
                }
            }else {
                #pragma omp parallel for
                for (int i = my_rank * stripe; i < (my_rank + 1) * stripe; i++) {
                    if (i == 0 || i == size - 1) continue;
                    for (int j = 1; j < size - 1; j++) {
                        z[t_cur][i][j] = 2.0 * z[t_1][i][j] - z[t_2][i][j] + p * (z[t_1][i + 1][j] + z[t_1][i - 1][j] + z[t_1][i][j + 1] + z[t_1][i][j - 1] - 4.0 * z[t_1][i][j]);
                    }
                }
            }
            
            // prints an intermediate status every interval(#) cycles
            if (interval != 0 && t % interval == 0) {
                // aggregate different partitional results from all rank
                if (my_rank == 0) { // receive all result from other rank
                    for (int rank = 1; rank < mpi_size; rank++) {
                        MPI_Recv(&(z[t_cur][rank * stripe][0]), size * stripe, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, &status);
                    }
                    
                    // after receive all result and print
                    cout << t << endl;
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < size; j++) {
                            cout << z[t_cur][i][j] << " " ;
                        }
                        cout << endl;
                    }
                    cout << endl;
                       
                } else {// all slave rank send data to rank0;
                    MPI_Send(&(z[t_cur][my_rank * stripe][0]), size * stripe, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
                }
            }
                
      } // end of simulation
        
      // finish the timer
        if (my_rank == 0) {
            cerr << "Elapsed time = " << time.lap( ) << endl;
        }
              
        MPI_Finalize();
        
      return 0;
    }





