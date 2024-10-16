/**
 * @file schedbench.c
 * @brief User benchmark program for a scheduler project targeting big-little architecture.
 *
 * This program creates multiple child processes using fork() and prints the time (tick) and CPU ID from when each child process starts until it ends.
 *
 * There are 8 types of tasks, and each child process performs one of these tasks.
 * Task 0: CPU-intensive (LONG_TIME)
 * Task 1: CPU-intensive (SHORT_TIME)
 * Task 2: Sleep-intensive (LONG_TIME*3)
 * Task 3: Sleep-intensive (SHORT_TIME)
 * Task 4: Sleep and CPU-intensive (1 iteration, LONG_TIME sleep, LONG_TIME/2 CPU-intensive)
 * Task 5: Sleep and CPU-intensive (30 iterations, 2 ticks sleep, 8 ticks CPU-intensive)
 * Task 6: Sleep and CPU-intensive (30 iterations, 8 ticks sleep, 2 ticks CPU-intensive)
 * Task 7: File read-intensive (read 'usertests' file 10 times)
 *
 * The PRINT macro is an option that provides immediate visual feedback on the start and end times of each child process. When enabled, it prints these times directly to stdout as soon as each child process is created and terminated.
 */
#include "types.h"
#include "stat.h"
#include "user.h"

#define SHORT_TIME  10
#define LONG_TIME 	300
#define NUM_TASK	8
//#define PRINT

// Simulate CPU-intensive workload
void
do_compute(int time)
{
	int a;
	int work = 0;
	while (work < time) {
		a = uptime();
		while (uptime() == a) {;}
		work++;
	}
	return;
}

// Simulate sleep-intensive workload
void
do_sleep(int time)
{
	sleep(time);
	return;
}

// Simulate a mix of sleep and CPU-intensive workload
void 
do_sleep_and_compute(int num_iter, int sleep_time, int compute_time)
{
	int a;
	int work;
	for (int iter=0; iter<num_iter; iter++) {
		sleep(sleep_time);

		work = 0;
		while (work < compute_time) {
			a = uptime();
			while (uptime() == a) {;}
			work++;
		}
	}
	return;
}

// Simulate file read-intensive workload
void 
do_fileread(void) 
{
	int fd;
	char buf[2048];
	int read_times = 10;
	int bytes;
	int totalbytes = 0;
	for (int i=0; i<read_times; i++) {
		fd = open("usertests", 0); // read a binary file
		if (fd >= 0) {
			while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
				totalbytes += bytes;
			}
			close(fd);
		} else {
			printf(1, "read failed\n");
		}
	}
}

// Get the user option and return total processes
int 
get_user_option_and_total_procs() 
{
    int option;
    char buf[100];
    printf(1, "1 (For project submission): Create 3 processes for each task 0-7\n");
    printf(1, "2 (For debugging): Create 1 process for each task 0-7\n");
    printf(1, "3 (Custom): Create N processes for each task 0-7\n");
    printf(1, "Select an option (1-3): ");
    gets(buf, sizeof(buf));
    option = atoi(buf);

    int total_procs;
    switch (option) {
    case 1:
        total_procs = 3 * NUM_TASK; // 3 processes for each of 8 tasks
        break;
    case 2:
        total_procs = NUM_TASK; // 1 process for each of 8 tasks
        break;
    case 3:
        printf(1, "Input N: ");
        gets(buf, sizeof(buf));
        total_procs = atoi(buf) * NUM_TASK;
        break;
    default:
        printf(1, "Invalid option. Exit\n");
        exit();
    }
    printf(1, "total_procs: %d\n", total_procs);
    return total_procs;
}

int
main(int argc, char *argv[])
{
    // Parent process timing
    int p_begin, p_end;

    // Child process timing and CPU IDs
    int c_begin, c_end;
    int begin_cpuid, end_cpuid;

    // Response and turnaround times
    int rsp, tr;
    int c_rsp_time = 0;
    int c_tr_time = 0;

    // Child process management
    int pid[64];
    int f_begin;

    // Inter-process communication
    int fds[64][2];

	// Others
    int i;
	int total_procs = get_user_option_and_total_procs();

	for (i=0; i<total_procs; i++) {
		if (pipe(fds[i]) == -1) {
			printf(1,"pipe err in pipe %d. increase NOFILE in param.h\n", i);
			exit();
		}
	}
	printf(1, "schedbench start\n");

	// Start timing the parent process
	p_begin = uptime();

	// Create child processes and assign them tasks
	for (i=0; i<total_procs; i++) {
		f_begin = uptime();
		pid[i] = fork();
		if (pid[i] == 0) {
			// This is the child process
			close(fds[i][0]);
			c_begin = uptime();

			// Get and store the CPU ID of the child process
			begin_cpuid = getcid();
			write(fds[i][1], &begin_cpuid, sizeof(begin_cpuid));
			write(fds[i][1], &c_begin, sizeof(c_begin));
#ifdef PRINT
			printf(1, "(cpu %d) child [%d] begin: %d\n", begin_cpuid, i, c_begin);
#endif

			// Assign a task to the child process based on its index
			switch (i%NUM_TASK) {
			case 0:
				do_compute(LONG_TIME); // heavy
				break;
			case 1:
				do_compute(SHORT_TIME); // light
				break;
			case 2:
				do_sleep(LONG_TIME*3); // light
				break;
			case 3:
				do_sleep(SHORT_TIME); // light
				break;
			case 4:
				do_sleep_and_compute(1, LONG_TIME, LONG_TIME/2); // heavy
				break;
			case 5:
				do_sleep_and_compute(30, 2, 8); // heavy
				break;
			case 6:
				do_sleep_and_compute(30, 8, 2); // light
				break;
			case 7:
				do_fileread(); // light
				break;
			default:
				break;
			}

			// End timing the child process
			c_end = uptime();
#ifdef PRINT
			printf(1, "child [%d] end: %d\n", i, c_end);
#endif
			// Calculate response and turnaround times
			rsp = c_begin - f_begin;
			tr = c_end - f_begin;

			// Get and store the CPU ID of the child process at the end of its execution
			end_cpuid = getcid();
			write(fds[i][1], &c_end, sizeof(c_end));
			write(fds[i][1], &rsp, sizeof(rsp));
			write(fds[i][1], &tr, sizeof(tr));
			write(fds[i][1], &end_cpuid, sizeof(end_cpuid));
			close(fds[i][1]);

			exit(); /* Task (child) exit */
		}
	}

	// Wait for all child processes to finish
	for (i=0; i<total_procs; i++)
		wait();

	// End timing the parent process
	p_end = uptime();
	printf(1, "Time from fork() to wait(): %d\n", p_end-p_begin);

	// Read and print the timing and CPU ID information of each child process using custom syscall (getcid)
	for (i=0; i<total_procs; i++) {
        read(fds[i][0], &begin_cpuid, sizeof(begin_cpuid));
        read(fds[i][0], &c_begin, sizeof(c_begin));
        read(fds[i][0], &c_end, sizeof(c_end));
        read(fds[i][0], &rsp, sizeof(rsp));
        read(fds[i][0], &tr, sizeof(tr));
        read(fds[i][0], &end_cpuid, sizeof(end_cpuid));
        close(fds[i][0]);

		// Calculate total response and turnaround times
		c_rsp_time += rsp;
        c_tr_time += tr;

        printf(1, "(%d) [cpu %d->%d] [tick %d->%d] response time: %d, turaround time: %d\n", i, begin_cpuid, end_cpuid, c_begin, c_end, rsp, tr);
    }   
    printf(1, "(Child) Total response time: %d (avg: %d), Total turnaround time: %d (avg: %d)\n", c_rsp_time, c_rsp_time/total_procs, c_tr_time, c_tr_time/total_procs);

	printf(1, "schedbench end\n");
	exit();
}