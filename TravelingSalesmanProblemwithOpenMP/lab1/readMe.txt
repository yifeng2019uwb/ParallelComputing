Lab1 - Calculate PI using OpenMP with 2 different methods.

Method1: Using trapezoid rule;
When using OpenMP for this solution, all threads parallel execute the a loop:
Thread 1: 0 - N/nThreads;
Thread 2: N/nThreads - 2 * N/nThreads;
...
Thread N: (N - 1)/nThreads - N;

So it is obvious the performance using OpenMP is much better than without it. Ideally, the performance improvement with threads using OpenMP should be about 3 times larger than without using OpenMP.

From my code, the performance improvement = 17105/6164 = 2.78 times.


[12:32:45] yifengzh_css534@cssmpi3: ~/lab1_1/final $ ./pi_integral
Etner the number of iterations used to estimate pi: 1000000
elapsed time for pi = 17105
# of trials = 1000000, estimate of pi is  3.1415926535897643, Error is 0.0000000000000289
[12:33:04] yifengzh_css534@cssmpi3: ~/lab1_1/final $ ./pi_integral_omp 
Etner the number of iterations used to estimate pi: 1000000
Etner the number of threads: 3
elapsed time for pi = 6164
# of trials = 1000000, estimate of pi is  3.1415926535899041, Error is 0.0000000000001110


Methods2: Using Monte Carlo method
This method using rand() function in the loop and rand() function will create the same sequence number. In order to avoid the same rand() function behavior, should initialize random numbers generator in each parallel thread with various values. So the performance with OpenMP is not improve much than without using OpenMP.

[12:33:35] yifengzh_css534@cssmpi3: ~/lab1_1/final $ ./pi_monte
Enter the number of iterations used to estimate pi: 1000000
elapsed time for pi = 108056
# of trials = 1000000, estimate of pi is  3.1400229999999998, Error is 0.0015696535897933
[12:33:43] yifengzh_css534@cssmpi3: ~/lab1_1/final $ ./pi_monte_omp
Enter the number of iterations used to estimate pi: 1000000
Enter the number of threads: 3
1 : quad = 2
2 : quad = 3
0 : quad = 0
0 : quad = 1
elapsed time for pi = 887347
# of trials = 1000000, estimate of pi is  3.1400749999999999, Error is 0.0015176535897932