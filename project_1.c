#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <sched.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

extern void init_scheduler(int);
extern int scheduleme(float, int, int, int);

FILE *fd;

struct _thread_info
{
	int id;
	float arrival_time;
	int required_time;
	int priority;
};
typedef struct _thread_info _thread_info_t;


double _global_time;
float _last_event_time;

pthread_mutex_t _time_lock;
pthread_mutex_t _last_event_lock;

void set_last_event(float last_event)
{
	pthread_mutex_lock(&_last_event_lock);
		if (last_event > _last_event_time)
			_last_event_time = last_event;
	pthread_mutex_unlock(&_last_event_lock);
}

float get_global_time()
{
	return _global_time;
}

void advance_global_time(float next_arrival)
{
	pthread_mutex_lock(&_time_lock);
		float next_ms = floor(_global_time + 1.0);
		if ((next_arrival < next_ms) && (next_arrival > 0))
			_global_time = next_arrival;
		else
			_global_time = next_ms;
	pthread_mutex_unlock(&_time_lock);
}

float read_next_arrival(float *arrival_time, int *id, int *required_time, int *priority)
{
	*arrival_time = -1.0;
	char c[15];
	fscanf(fd, "%f.1", arrival_time);
	fgets(c, 1, fd);
	fscanf(fd, "%d", id);
	fgets(c, 1, fd);
	fscanf(fd, "%d", required_time);
	fgets(c, 1, fd);
	fscanf(fd, "%d", priority);
	fgets(c, 10, fd);

	return *arrival_time;
}

int open_file(char *filename)
{
	char mode = 'r';
	fd = fopen(filename, &mode);
	if (fd == NULL)
	{
		printf("Invalid input file specified: %s\n", filename);
		return -1;
	}
	else
	{
		return 0;
	}
}

void close_file()
{
	fclose(fd);
}

void *worker_thread(void *arg)
{
	_thread_info_t *myinfo = (_thread_info_t *)arg;
	float time_remaining = myinfo->required_time * 1.0;
	float scheduled_time;
	set_last_event(myinfo->arrival_time);
	scheduled_time = scheduleme(myinfo->arrival_time, myinfo->id, time_remaining, myinfo->priority);

	while (time_remaining > 0)
	{
		set_last_event(scheduled_time);
		//printf("T%d\n", myinfo->id);
		printf("%3.0f - %3.0f: T%d\n", scheduled_time, scheduled_time + 1.0, myinfo->id);
		while(get_global_time() < scheduled_time + 1.0)
		{
			sched_yield();
		}
		time_remaining -= 1.0;
		scheduled_time = scheduleme(get_global_time(), myinfo->id, time_remaining, myinfo->priority); 
											// Removed F from get_global_Ftime() 
	}
	free(myinfo);
	pthread_exit(NULL);
}

int _pre_init(int sched_type)
{
	pthread_mutex_init(&_time_lock, NULL);
	pthread_mutex_init(&_last_event_lock, NULL);
	init_scheduler(sched_type);

	_global_time = 0.0;
	_last_event_time = -1.0;
}

int main(int argc, char *argv[])
{	argc = 3;// ./out 0 input_1
	argv[1] = "0";// schedule type
	argv[2] = "input_0";// input file
	int inactivity_timeout = 50;
	if (argc < 3)
	{
		printf ("Not enough parameters specified.  Usage: a.out <scheduler_type> <input_file>\n");
		printf ("  Scheduler type: 0 - First Come, First Served (Non-preemptive)\n");
		printf ("  Scheduler type: 1 - Shortest Remaining Time First (Preemptive)\n");
		printf ("  Scheduler type: 2 - Priority-based Scheduler (Preemptive)\n");
		printf ("  Scheduler type: 3 - Multi-Level Feedback Queue w/ Aging (Preemptive)\n");
		return -1;
	}

	if (open_file(argv[2]) < 0)
		return -1;

	_pre_init(atoi(argv[1]));

	int this_thread_id = 0;

	_thread_info_t *ti;

	float next_arrival_time;
	pthread_t pt;

	ti = (_thread_info_t *)malloc(sizeof(_thread_info_t));
	next_arrival_time = read_next_arrival(&(ti->arrival_time), &(ti->id), &(ti->required_time), &(ti->priority));
	if (next_arrival_time < 0)
		return -1;

	pthread_create(&pt, NULL, worker_thread, ti);

	while (_last_event_time != ti->arrival_time)
		sched_yield();

	ti = (_thread_info_t *)malloc(sizeof(_thread_info_t));
	next_arrival_time = read_next_arrival(&(ti->arrival_time), &(ti->id), &(ti->required_time), &(ti->priority));
	while ((get_global_time() - _last_event_time) < inactivity_timeout)
	{
		advance_global_time(next_arrival_time);		// Advance timer to next whole unit, or event
		if (get_global_time() == next_arrival_time)
		{
			pthread_create(&pt, NULL, worker_thread, ti);

			while (_last_event_time < ti->arrival_time)
			{
				sched_yield();
			}
			ti = (_thread_info_t *)malloc(sizeof(_thread_info_t));
			next_arrival_time = read_next_arrival(&(ti->arrival_time), &(ti->id), &(ti->required_time), &(ti->priority));

			if (next_arrival_time < 0)
			{
				free(ti);
			}
		}
		else
		{
			int loop_counter = 0;
			while ((_last_event_time < get_global_time()) && (loop_counter < 100000))
			{
				loop_counter++;
				sched_yield();
			}
		}
	}

	close_file();
	return 0;
}
