#include <iostream>
#include "Timer.h"
#include <stdlib.h>   // atoi
#include <math.h>       // pow


int default_size = 100;  // the default system size
int defaultCellWidth = 8;
double c = 1.0;      // wave speed
double dt = 0.1;     // time quantum
double dd = 2.0;     // change in system
bool printing = false;

using namespace std;

int main( int argc, char *argv[] ) {
    // verify arguments
    if ( argc != 4 ) {
        cerr << "usage: Wave2D size max_time interval" << endl;
        return -1;
    }
    int size = atoi( argv[1] );   // the edge length of a 2D simulated square
    int max_time = atoi( argv[2] );   // # steps to simulate wave dissmination(>=2)
    int interval  = atoi( argv[3] );  // #simulation steps needed each time the current simulation sapce is printed out
    
    if ( size < 100 || max_time < 3 || interval < 0 ) {
        cerr << "usage: Wave2D size max_time interval" << endl;
        cerr << "       where size >= 100 && time >= 3 && interval >= 0" << endl;
        return -1;
    }
    
    double factor = pow(c, 2) * pow((double)(dt/dd), 2);
    
    // create a simulation space
    double z[3][size][size];
    for ( int p = 0; p < 3; p++ )
        for ( int i = 0; i < size; i++ )
            for ( int j = 0; j < size; j++ )
                z[p][i][j] = 0.0; // no wave
    
    // start a timer
    Timer time;
    time.start( );
    
    // time = 0;
    // initialize the simulation space: calculate z[0][][]
    int weight = size / default_size;
    for( int i = 0; i < size; i++ ) {
        for( int j = 0; j < size; j++ ) {
            if( i > 40 * weight && i < 60 * weight  &&
               j > 40 * weight && j < 60 * weight ) {
                z[0][i][j] = 20.0;
            } else {
                z[0][i][j] = 0.0;
            }
        }
    }
    
    if (printing) {
        cout << "0" << endl;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                cout << z[0][i][j] << " " ;
            }
            cout << endl;
        }
        cout << endl;
    }
    
    // time = 1
    // calculate z[1][][]
    // cells not on edge: for (i = 0, i = N -1, j = 0, or j = N – 1), Zt_i,j = 0.0
    // IMPLEMENT BY YOURSELF !!!
    // formula when t = 1
    // Zt_i,j = Zt-1_i,j + c^2 / 2 * (dt/dd)2 * (Zt-1_i+1,j + Zt-1_i-1,j + Zt-1_i,j+1 + Zt-1_i,j-1 – 4.0 * Zt-1_i,j)
    
    for (int i = 1; i < size - 1; i++) {
        for (int j = 1; j < size - 1; j++) {
            z[1][i][j] = z[0][i][j] + (factor / 2.0) * (z[0][i + 1][j] + z[0][i - 1][j] + z[0][i][j + 1] + z[0][i][j - 1] - 4.0 * z[0][i][j]);
        }
    }
    
    if (printing) {
        cout << "1" << endl;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                cout << z[1][i][j] << " " ;
            }
            cout << endl;
        }
        cout << endl;
    }
    
    // simulate wave diffusion from time = 2
    /*
     Schroedinger’s wave formula computes Zt_i,j (where t >= 2 ) as follows:
     Zt_i,j = 2.0 * Zt-1_i,j – Zt-2_i,j + c^2 * (dt/dd)2 * (Zt-1_i+1,j + Zt-1_i-1,j + Zt-1_i,j+1 + Zt-1_i,j-1 – 4.0 * Zt-1_i,j)
     */
    
    int t_cur = 2, t_1 = 1, t_2 = 0;
    for ( int t = 2; t < max_time; t++ ) {
        // IMPLEMENT BY YOURSELF !!!
        // shift down / rotate z[2][][], z[1][][], and z[0][][].
        
        
        t_cur = t % 3;  // t_cur = (t_cur + 1) % 3;
        t_1 = (t - 1) % 3;    // t_1 = (t_1 + 1) % 3;
        t_2 = (t - 2) % 3;    // t_2 = (t_2 + 1) % 3;
        
        for (int i = 1; i < size - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                z[t_cur][i][j] = 2.0 * z[t_1][i][j] - z[t_2][i][j] + factor * (z[t_1][i + 1][j] + z[t_1][i - 1][j] + z[t_1][i][j + 1] + z[t_1][i][j - 1] - 4.0 * z[t_1][i][j]);
            }
        }
        
        // prints an intermediate status every interval(#) cycles
        if (interval != 0 && t % interval == 0) {
            //print out
            // cout << "Time = " << t << endl;
            cout << t << endl;
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    cout << z[t_cur][i][j] << " " ;
                }
                cout << endl;
            }
            cout << endl;
        }
        
    } // end of simulation
    
    // finish the timer
    cerr << "Elapsed time = " << time.lap( ) << endl;
    return 0;
}


