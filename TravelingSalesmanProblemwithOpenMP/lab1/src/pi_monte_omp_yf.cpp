// Program to compute Pi using Monte Carlo methods
// Original code availabe from Dartmath through Internet
// Code modified to compute all four quadrants

#include "Timer.h"    // for performance measurement

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#define SEED 35791246

using namespace std;
double helper(int numbers);

int main(int argc, char* argv[]) {
    int niter=0;
    double PI25DT = 3.141592653589793238462643; // the actual PI
    double pi = 0.0;
    int nThreads, thread;
    
    cout << "Enter the number of iterations used to estimate pi: ";
    cin >> niter;
    cout << "Enter the number of threads: ";
    cin >> nThreads;
    
    Timer timer;
    timer.start( );
    
    
    srand( SEED );  // initialize random numbers
    #pragma omp parallel num_threads(nThreads)
    {
        double x,y, radius;
        #pragma omp for
        for ( int quad = 0; quad < 4; quad++ ) {
            // for each quadrant
            cout << omp_get_thread_num() << " : quad = " << quad << endl;
            int count = 0;
            #pragma omp parallel for reduction(+:count)
            for ( int i = 0; i < niter; i++ ) {
                x = ( double )rand( ) / RAND_MAX;
                y = ( double )rand( ) / RAND_MAX;
                radius =  x * x + y * y;
                if ( radius <= 1 ) count++;
            }
            pi += ( double )count / niter;
        }
    }
    
    cout << "elapsed time for pi = " << timer.lap( ) << endl;
    
    printf( "# of trials = %d, estimate of pi is  %.16f, Error is %.16f\n",
           niter, pi, fabs( pi - PI25DT ) );
    /*
     omp_set_num_threads(nThreads);
     #pragma omp parallel default(none)
     {
     for (int i = 0; i < 5; i++ ) {
     printf("hello, from thread #%d ! \n", omp_get_thread_num());
     }
     
     }
     */
    return 0;
}

double helper(int numbers) {
    
    double x, y;   // x-y coordinates in each quadrant
    double radius;
    srand( SEED );  // initialize random numbers
    
    int count = 0;

    #pragma omp parallel for private(x, y) reduction(+:count)
    for ( int i = 0; i < numbers; i++ ) {
        x = ( double )rand( ) / RAND_MAX;
        y = ( double )rand( ) / RAND_MAX;
        radius =  x * x + y * y;
        if ( radius <= 1 ) count++;
    }
    cout << "count/numbers = " << count/numbers;
    return count/numbers;
}
