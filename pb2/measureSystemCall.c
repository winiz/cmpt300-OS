#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
/* Your Answers Here
2.  a) cloud9. Its a Server.
    b) 2.6 - 3.0 ms
    c) 3.0 ms / 1,000,000 times = 3,000,000ns / 1,000,000 times = 3 ns/pass
    
3.  a)3.4 ms - 3.9ms
    b)3.4 ns - 3.9ns
    
2.  a) 63.3 ms - 67.7 ms
    b) 63.3 ns - 67.7 ns
    
13. a)8.9 ms - 11.4 ms for 100 iterations
        89.5ms - 96.8 ms for 1000 iterations
    b)using the data for 100 iterations
        89,000 ns - 114,000 ns
    c) For every loop, the fork() is excuted twice, 
    if fork() = 0, the child process is created then the program enters the next loop
    elif fork() = 1, do nothing 
    else there is a error, increments the error counter and wait untill its child process teminates
*/
double timespec_to_ms(struct timespec *ts){
    return ts->tv_sec*1000.0 + ts->tv_nsec/1000000.0;
}

int main(){
    struct timespec start_time, end_time;
    int i = 0;
    int errorCount = 0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    
    /* begin timing */
    for (i;i<1000;i++){
        int pid = fork();
        if (pid == 0) {
            // only child process executes this
            return 0;
            
        }
        if (pid < 0) {
            // error is generated
        printf("ERROR: Fork failed.\n");
        errorCount++;
        wait(0);
        }
    }
    
    /* end timing */
    
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    printf("%f ms\n", timespec_to_ms(&end_time)-timespec_to_ms(&start_time));
}