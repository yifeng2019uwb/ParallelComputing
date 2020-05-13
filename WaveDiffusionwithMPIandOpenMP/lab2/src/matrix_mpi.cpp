// #include "mpi.h"
#include <stdlib.h> // atoi
#include <iostream> // cerr
#include <mpi.h>    // mpi
#include "Timer.h"

using namespace std;

void init( double *matrix, int size, char op ) {
  for ( int i = 0; i < size; i++ )
    for ( int j = 0; j < size; j++ )
      matrix[i * size + j]
	= ( op == '+' ) ? i + j :
	( ( op == '-' ) ? i - j : 0 );
}

void print( double *matrix, int size, char id ) {
  for ( int i = 0; i < size; i++ )
    for ( int j = 0; j < size; j++ )
      cout << id << "[" << i << "][" << j << "] = " << matrix[i * size + j] << endl;
}

void multiplication( double *a, double *b, double *c, int stripe, int size ) {
  for ( int k = 0; k < size; k++ )
    for ( int i = 0; i < stripe; i++ )
      for ( int j = 0; j < size; j++ )
	// c[i][k] += a[i][j] * b[j][k];
	c[i * size + k] += a[i * size + j] * b[j * size + k];
}

int main( int argc, char* argv[] ) {
  int my_rank = 0;            // used by MPI
  int mpi_size = 1;           // used by MPI
  int size = 400;             // array size
  bool print_option = false;  // print out c[] if it is true
    
    int source, dest, tag = 0;
    MPI_Status status;
    
  Timer timer;

  // variables verification
  if ( argc == 3 ) {
    if ( argv[2][0] == 'y' )
      print_option = true;
  }
  
  if ( argc == 2 || argc == 3 ) {
    size = atoi( argv[1] );
  }
  else {
    cerr << "usage:   matrix size [y|n]" << endl;
    cerr << "example: matrix 400   y" << endl;
    return -1;
  }

  MPI_Init( &argc, &argv ); // start MPI
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );    // MPI_COMM_WORLD is set during Init()
                                                // get my process id
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );   // get the number of processes

  // matrix initialization
  double *a = new double[size * size];
  double *b = new double[size * size];
  double *c = new double[size * size];

  if ( my_rank == 0 ) { // master initializes all matrices
    init( a, size, '+' );
    init( b, size, '-' );
    init( c, size, '0' );

    // print initial values
    if ( false ) {
      print( a, size, 'a' );
      print( b, size, 'b' );
    }

    // start a timer
    timer.start( );
  }
  else {                // slavs zero-initializes all matrices
    init( a, size, '0' );
    init( b, size, '0' );
    init( c, size, '0' );
  }

  // broadcast the matrix size to all.
  //  source = 0;
   MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );

  int stripe = size / mpi_size;     // partitioned stripe

  // master sends each partition of a[] to a different slave
  // master also sends b[] to all slaves
    
    if (my_rank == 0) { // master
        for (int rank = 1; rank < mpi_size; rank++) {
            MPI_Send(a + rank * stripe * size, size * stripe, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD );
            MPI_Send(b, size * size, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD);
        }
    }else { // slave
        MPI_Recv(a, size * stripe, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(b, size * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
    }

  multiplication( a, b, c, stripe, size ); // all ranks should compute multiplication

  // master receives each partition of c[] from a different slave
    if (my_rank == 0) {
        for (int rank = 1; rank < mpi_size; rank++) {
            MPI_Recv(c + rank * stripe * size, size * stripe, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, &status);
        }
    } else {
        MPI_Send(c, stripe * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    

  if ( my_rank == 0 )
    // stop the timer
    cout << "elapsed time = " << timer.lap( ) << endl;

  // results
  if ( print_option && my_rank == 0 )
    print( c, size, 'c' );

   MPI_Finalize( ); // shut down MPI
}


  

  