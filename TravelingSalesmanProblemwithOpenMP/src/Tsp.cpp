#include <iostream>  // cout
#include <fstream>   // ifstream
#include <string.h>  // strncpy
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     // OpenMP
#include "Timer.h"
#include "Trip.h"

using namespace std;

// Already implemented. see the actual implementations below
void initialize( Trip trip[CHROMOSOMES], int coordinates[CITIES][2] );
void select( Trip trip[CHROMOSOMES], Trip parents[TOP_X] );
void populate( Trip trip[CHROMOSOMES], Trip offsprings[TOP_X] );

// need to implement for your program 1
void setDistance(float distance[CITIES][CITIES], int coordinates[CITIES][2] );
void evaluate( Trip trip[CHROMOSOMES], float distance[CITIES][CITIES] );
void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], float distance[CITIES][CITIES] );
void mutate( Trip offsprings[TOP_X] );

int getIndex(char c);
char getCityName(int n);
void generateChild(Trip parents[TOP_X], Trip offsprings[TOP_X], int i, float distance[CITIES][CITIES]);
void generateComplementChild(Trip offsprings[TOP_X] , int i);
int getNextCityPosition(Trip parent, float distance[CITIES][CITIES], int currPosition, bool flag[CITIES]);
int findPosition(Trip parent, char cityName);
int getRandomCityIndex(bool flag[CITIES],  int num );
int offsetNumber = 0;

/*
 * MAIN: usage: Tsp #threads
 */
int main( int argc, char* argv[] ) {
    Trip trip[CHROMOSOMES];       // all 50000 different trips (or chromosomes)
    Trip shortest;                // the shortest path so far
    int coordinates[CITIES][2];   // (x, y) coordinates of all 36 cities:
    int nThreads = 1;
    float distance[CITIES][CITIES];
    
    // verify the arguments
    if ( argc == 2 )
        nThreads = atoi( argv[1] );
    else {
        cout << "usage: Tsp #threads" << endl;
        if ( argc != 1 )
            return -1; // wrong arguments
    }
    cout << "# threads = " << nThreads << endl;
    
    // shortest path not yet initialized
    shortest.itinerary[CITIES] = 0;  // null path
    shortest.fitness = -1.0;         // invalid distance
    
    // initialize 5000 trips and 36 cities' coordinates
    initialize( trip, coordinates );
    
    // start a timer
    Timer timer;
    timer.start( );
    
    // change # of threads
    // omp_set_dynamic(0);
    omp_set_num_threads( nThreads );
    
    // #pragma omp parallel default
    
    // Set distance[CITIES][CITIES] Matrix in floating type
    setDistance(distance, coordinates);
    
    // find the shortest path in each generation (MAX_GENERATION = 150)
    // #pragma omp parallel shared(distance, coordinates, trip)
    {
        
        for ( int generation = 0; generation < MAX_GENERATION; generation++ ) {
            
            // evaluate the distance of all 50000 trips
            evaluate( trip, distance );
            
            // just print out the progress
            if ( generation % 20 == 0 )
                cout << "generation: " << generation << endl;
            
            // whenever a shorter path was found, update the shortest path
            
            #pragma omp critical
            if ( shortest.fitness < 0 || shortest.fitness > trip[0].fitness ) {
                
                strncpy( shortest.itinerary, trip[0].itinerary, CITIES );
                shortest.fitness = trip[0].fitness;
                cout << "generation: " << generation
                << " shortest distance = " << shortest.fitness
                << "\t itinerary = " << shortest.itinerary << endl;
                
            }
            
            // define TOP_X parents and offsprings.
            Trip parents[TOP_X], offsprings[TOP_X];
            
            // choose TOP_X parents from trip
            select( trip, parents );
                      
            // generates TOP_X offsprings from TOP_X parenets
            crossover( parents, offsprings, distance );
            
            // mutate offsprings
            mutate( offsprings );
            
            // populate the next generation.
            populate( trip, offsprings );
            // if (generation == 0) break;
        }
        
    }
    
    // stop a timer
    cout << "elapsed time = " << timer.lap( ) << endl;
    return 0;
}

/*
 * Initializes trip[CHROMOSOMES] with chromosome.txt and coordiantes[CITIES][2] with cities.txt
 *
 * @param trip[CHROMOSOMES]:      50000 different trips
 * @param coordinates[CITIES][2]: (x, y) coordinates of 36 different cities: ABCDEFGHIJKLMNOPQRSTUVWXYZ
 */
void initialize( Trip trip[CHROMOSOMES], int coordinates[CITIES][2] ) {
    // open two files to read chromosomes (i.e., trips)  and cities
    ifstream chromosome_file( "chromosome.txt" );
    ifstream cities_file( "cities.txt" );
    
    // read data from the files
    // chromosome.txt:
    //   T8JHFKM7BO5XWYSQ29IP04DL6NU3ERVA1CZG
    //   FWLXU2DRSAQEVYOBCPNI608194ZHJM73GK5T
    //   HU93YL0MWAQFIZGNJCRV12TO75BPE84S6KXD
    
    #pragma omp for
    for (int i = 0; i < CHROMOSOMES; i++ ) {
        chromosome_file >> trip[i].itinerary;
        trip[i].fitness = 0.0;
    }
    
    // cities.txt:
    // name    x       y
    // A       83      99
    // B       77      35
    // C       14      64
    #pragma omp_parallel for
    {
        #pragma omp for
        for (int  i = 0; i < CITIES; i++ ) {
            char city;
            cities_file >> city;
            int index = ( city >= 'A' ) ? city - 'A' : city - '0' + 26;
            cities_file >> coordinates[index][0] >> coordinates[index][1];
        }
    }
    // close the files.
    chromosome_file.close( );
    cities_file.close( );
    
    // just for debugging
    if ( DEBUG ) {
        for (int  i = 0; i < CHROMOSOMES; i++ )
            cout << trip[i].itinerary << endl;
        for (int i = 0; i < CITIES; i++ )
            cout << coordinates[i][0] << "\t" << coordinates[i][1] << endl;
    }
}

/*
 * Select the first TOP_X parents from trip[CHROMOSOMES]
 *
 * @param trip[CHROMOSOMES]: all trips
 * @param parents[TOP_X]:    the firt TOP_X parents
 */
void select( Trip trip[CHROMOSOMES], Trip parents[TOP_X] ) {
    // just copy TOP_X trips to parents
#pragma omp_parallel for shared(trip, parents)
    {
#pragma omp for
        for ( int i = 0; i < TOP_X; i++ ) {
            strncpy( parents[i].itinerary, trip[i].itinerary, CITIES + 1 );
        }
    }
    
}

/*
 * Replace the bottom TOP_X trips with the TOP_X offsprings
 */
void populate( Trip trip[CHROMOSOMES], Trip offsprings[TOP_X] ) {
    // just copy TOP_X offsprings to the bottom TOP_X trips.
    
    #pragma omp parallel shared(trip, offsprings)
    {
        #pragma omp for
        for (int i = 0; i < TOP_X; i++ ){
            strncpy( trip[ CHROMOSOMES - TOP_X + i ].itinerary, offsprings[i].itinerary, CITIES + 1 );
        }
        
    }
    
    // for debugging
    if ( DEBUG ) {
        for ( int chrom = 0; chrom < CHROMOSOMES; chrom++ )
            cout << "chrom[" << chrom << "] = " << trip[chrom].itinerary
            << ", trip distance = " << trip[chrom].fitness << endl;
    }
    
}


/*
 * compare function for sort trips in shortest-first order
 */
int compareDistance(const void* a, const void* b) {
    
    float diff = ((Trip*) a)->fitness - ((Trip*) b)->fitness;
    if (diff > 0) {
        return 1;
    } else if (diff < 0) {
        return -1;
    } else {
        return 0;
    }
}


/*
 * Evaluates the distance of each trip and sorts out all the trips in the shortest-first order.
 * Memorize the current shortest trip as a tentative answer if it is shorter the previous.
 * @param trip[CHROMOSOMES]: all trips
 * @param distance[CITIES][CITIES]:    distance between any 2 cities(or cities and (0,0) )
 */
void evaluate( Trip trip[CHROMOSOMES], float distance[CITIES][CITIES] ){
    
    #pragma omp parallel for shared(distance, trip)
    for (int i = 0; i < CHROMOSOMES; i++) {
        // start from (0,0);
        trip[i].fitness = distance[getIndex(trip[i].itinerary[0])][getIndex(trip[i].itinerary[0])];
        for (int j = 1; j < CITIES; j++) {
            trip[i].fitness += distance[getIndex(trip[i].itinerary[j - 1])][getIndex(trip[i].itinerary[j])];
        }
    }
    
    // sort trip with compareDistance function in shortest-first order
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            qsort(trip, CHROMOSOMES, sizeof(Trip), compareDistance);
        }
    }
    
    // for debugging - tested successed
    if ( DEBUG ) {
        for ( int chrom = 0; chrom < CHROMOSOMES/100; chrom++ )
            cout << trip[chrom].fitness << " : " << trip[chrom].itinerary << endl;
    }
    
}


/*
 * generates TOP_X offsprings from TOP_X parenets
 * @param parents[TOP_X]:    the firt TOP_X parents
 * @param distance[CITIES][CITIES]   distance between any 2 cites of 36 different cities
 * @param coordinates[CITIES][2]     coordinates of 36 different cities:
 */
void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], float distance[CITIES][CITIES] ){
    // cout << "start crossover  : " << TOP_X << endl;
    #pragma omp parallel shared(parents, offsprings)
    {
        #pragma omp for
        for (int i = 0; i < TOP_X; i += 2) {
            generateChild(parents, offsprings, i, distance);
            //    #pragma omp section
            generateComplementChild(offsprings, i);
            
            // if (i == 0) break;
        }
    }
    
}


/*
 * mutate(): randomly choose two distinct cities(or genes) in each trip(or chromosome)
 * with a given probability, and swaps them
 * @param offsprings[TOP_X]:    the firt TOP_X offspring
 */
void mutate( Trip offsprings[TOP_X] ){
    // cout << "mutate is called! " << endl;
    //  #pragma omp parallel shared(offsprings)
    {
        #pragma omp for
        for (int i = 0; i < TOP_X; i++) {
            if (rand() % 100 < MUTATE_RATE) {
                int city1 = rand() % CITIES;
                int city2 = rand() % CITIES;
                while (city1 == city2) {
                    // city2 = rand() % CITIES;
                    city2 = (city2 + 1) % CITIES;
                }
                swap(offsprings[i].itinerary[city1], offsprings[i].itinerary[city2]);
            }
        }
    }
}


/*
 * calculate the distance between any two cities
 * @param distance[CITIES][CITIES]   distance between any 2 cites of 36 different cities
 * @param coordinates[CITIES][2]     coordinates of 36 different cities:
 */
void setDistance(float distance[CITIES][CITIES], int coordinates[CITIES][2] ) {
    memset(distance, 0, sizeof(distance));
    float dist = 0.0;
    
    //   #pragma omp parallel shared(coordinates, distance)
    {
        #pragma omp parallel for
        for (int i = 0; i < CITIES; i++) {
            // distance at same city should be 0; but considering a travel start from (0,0),
            // I use the distance between the city-i to (0,0) instead of 0;
            // distance[i][i] = sqrt(pow(coordinates[i][0]), 2) + pow(coordinates[i][1]), 2);
            distance[i][i] = sqrt(coordinates[i][0] * coordinates[i][0] + coordinates[i][1] * coordinates[i][1]);
            for (int j = i + 1; j < CITIES; j++) {
                int dx = coordinates[i][0] - coordinates[j][0];
                int dy = coordinates[i][1] - coordinates[j][1];
                dist = sqrt(dx * dx + dy * dy);
                distance[i][j] = dist;
                distance[j][i] = dist;
                // cout << distance[i][j] << " ";
            }
            // cout << endl;
        }
    }
    
}


/*
 * convert cities char sign/name to city index number
 */
int getIndex(char c) {
    return c >= 'A' ? c - 'A' : c - '0' + 26;
}

/*
 * convert cities index to city Name
 */
char getCityName(int n) {
    if (n > 25) n += 'A';
    else n = '0'  + n - 26;
    char c = static_cast<char>(n);
    return c;
}

/*
 * Crossover Algorithm - generates child[i]
 * We select the first city of parent[i], compares the cities leaving that city in parent[i] and [i+1],
 * and chooses the closer one to extend child[i]’s trip. If one city has already appeared in the trip,
 * we choose the other city. If both cities have already appeared, we randomly select a non-selected city.
 * Thereafter, we generate child[i+1]’s trip as a complement of child[i].
 */
void generateChild(Trip parents[TOP_X], Trip offsprings[TOP_X],  int i, float distance[CITIES][CITIES]){
    // cout << "generateChild called  - " << parents[i].itinerary << endl ;
    bool flag[CITIES] = {};
    // start from parents[i].itinerary[0];
    char currCity = parents[i].itinerary[0], nextCity;
    int currPosition[2]; // current position in the parents[i] & parents[i + 1]
    currPosition[0] = 0;
    currPosition[1] = findPosition(parents[i + 1], currCity);
    
    offsprings[i].itinerary[0] = currCity;
    
    for (int j = 1; j < CITIES; j++) {
        flag[getIndex(currCity)] = true;
        //  char first_nextCity = 0, second_nextCity = 0;
        int first_nextCityPosition = getNextCityPosition(parents[i], distance, currPosition[0], flag);
        int second_nextCityPosition = getNextCityPosition(parents[i + 1], distance, currPosition[1], flag);
        
        
        if (first_nextCityPosition < 0 && second_nextCityPosition < 0) {
            // random select a non-selected city
            int nextCityIndex = getRandomCityIndex(flag, j);
            nextCity = INDEXTOCITYNAMETABLE[nextCityIndex];
            // need find the poistion for nextCity in parents[i] and parents[i + 1]
            currPosition[0] = findPosition(parents[i], nextCity);
            currPosition[1] = findPosition(parents[i + 1], nextCity);
        }else {
            //  cout << "fisrt : " << first_nextCityPosition << " : " << parents[i].itinerary[first_nextCityPosition] << endl ;
            // cout << "Second : " << second_nextCityPosition << " : " << parents[i + 1].itinerary[second_nextCityPosition]  << endl;
            if (first_nextCityPosition < 0) {
                nextCity = parents[i + 1].itinerary[second_nextCityPosition];
                currPosition[1] = second_nextCityPosition;
                currPosition[0] = findPosition(parents[i], nextCity);
            }else if (second_nextCityPosition < 0) {
                nextCity = parents[i].itinerary[first_nextCityPosition];
                currPosition[0] = first_nextCityPosition;
                currPosition[1] = findPosition(parents[i + 1], nextCity);
            }else {
                if (distance[getIndex(currCity)][getIndex(parents[i].itinerary[first_nextCityPosition])] < distance[getIndex(currCity)][getIndex(parents[i + 1].itinerary[second_nextCityPosition])]) {
                    nextCity = parents[i].itinerary[first_nextCityPosition];
                    currPosition[0] = first_nextCityPosition;
                    currPosition[1] = findPosition(parents[i + 1], nextCity);
                }else {
                    nextCity = parents[i + 1].itinerary[second_nextCityPosition];
                    currPosition[1] = second_nextCityPosition;
                    currPosition[0] = findPosition(parents[i], nextCity);
                }
            }
        }
        offsprings[i].itinerary[j] = nextCity;
        currCity = nextCity;
        // cout << currCity <<  ;
    }
}


/*
 * Crossover Algorithm - getRandomCityIndex: using greedy crossover algorithms
 * Using 2 solutions to get random number
 * using rand() function to generate random number, since rand() is sequential,
 * multi-threads doesn't help to improve the performance
 * Using global variable offsetNunber plus the current data 2D position-it's aways different number
 * solution2 has better performance than solution1
*/
int getRandomCityIndex(bool flag[CITIES], int num ) {
    // Solution 1: Use rand() which is sequential, not good for multi-threads
    //srand(int(time(NULL)) ^ omp_get_thread_num());
    // int selectedcityIndex = rand() % CITIES;
    
    // Solution 2: using offsetNumber + data 2D position
    int selectedcityIndex = (num + offsetNumber) % CITIES;
    offsetNumber++;
    
    while (flag[selectedcityIndex]) {
        selectedcityIndex = (selectedcityIndex + 1)% CITIES;
    }
    
    return selectedcityIndex;
}

int getNextCityPosition(Trip parent, float distance[CITIES][CITIES], int currPosition, bool flag[CITIES]){
    float dist = -1;
    int nextCityPosition = -1;
    int temp = (currPosition - 1 + CITIES) % CITIES;
    if ( !flag[getIndex(parent.itinerary[temp])] ){
        nextCityPosition = temp;
        dist = distance[getIndex(parent.itinerary[temp])][getIndex(parent.itinerary[currPosition])];
    }
    
    temp = (currPosition + 1) % CITIES;
    if ( !flag[getIndex(parent.itinerary[temp])] && (dist < 0 || dist > distance[getIndex(parent.itinerary[temp])][getIndex(parent.itinerary[currPosition])])) {
        nextCityPosition = temp;
        dist = distance[getIndex(parent.itinerary[temp])][getIndex(parent.itinerary[currPosition])];
    }
    return nextCityPosition;
}


/*
 * Crossover Algorithm - generates child[i + 1] using a complement of child[i]
 */
void generateComplementChild(Trip offsprings[TOP_X], int i){
    //   char complementMap[CITIES] = {'9','8','7','6','5','4','3','2','1','0','Z','Y','X','W','V','U','T','S','R','Q','P','O','N','M','L','K','J','H','G','F','E','D','C','B','A'} ;
#pragma omp_parallel for
    for (int j = 0; j < CITIES; j++) {
        offsprings[i + 1].itinerary[j]= INDEXTOCITYNAMETABLE[CITIES - 1 - getIndex(offsprings[i].itinerary[j])];
    }
    
}


int findPosition(Trip parent, char cityName) {
    for (int k = 0; k < CITIES; k++) {
        if (parent.itinerary[k] == cityName) {
            return k;
            
        }
    }
    return -1;
}






