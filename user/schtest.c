#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

#define NCHILD 10

void cpu_work() {
    volatile int x = 0;
    for (int i = 0; i < 100000000; i++) {
        x += i * (i + 1);
    }
}

void io_work() {
    for (int i = 0; i < 10; i++) {
        sleep(2);  // Sleep to simulate waiting for I/O
    }
}

int main(void) {
    int n, pid, status;

    printf("Starting scheduler benchmark...\n");

    for (n = 0; n < NCHILD; n++) {
        pid = fork();
        if (pid < 0) {
            printf("Fork failed\n");
            exit(1);
        }
        if (pid == 0) {  // Child process
            if (n % 2 == 0)
                cpu_work();  // Even children are CPU-bound
            else
                io_work();   // Odd children are I/O-bound
            exit(0);
        }
    }

    uint rtime, wtime;

    for (n = 0; n < NCHILD; n++) {
        pid = waitx((uint64)&status, &rtime, &wtime);
        if (pid < 0) {
            printf("Error waiting for child\n");
            continue;
        }
        printf("Process %d (%s) completed in %d ticks with %d ticks of waiting\n",
               n + 1, (n % 2 == 0) ? "CPU-bound" : "I/O-bound", rtime, wtime);
    }

    exit(0);
}
