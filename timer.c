#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<errno.h>
#include <unistd.h>

#if 0

This information is just added for rerefence. Like a comment.

           struct sigaction {
              /* u can use it or u can use the below sa_sigaction. if u do not need void* to send data, then u can use it.
                 u can use sa_handler = timer_cb; */
               void     (*sa_handler)(int);
               void     (*sa_sigaction)(int, siginfo_t *, void *);
               sigset_t   sa_mask;
               int        sa_flags;
               void     (*sa_restorer)(void);
           };


This structure tells how the process should be notified when the event(hence the name:sigevent) occurs.
       struct sigevent {
           int    sigev_notify;  /* Notification method */
           int    sigev_signo;   /* Notification signal */
           union sigval sigev_value;
                                 /* Data passed with notification */
           void (*sigev_notify_function)(union sigval);
                                 /* Function used for thread
                                    notification (SIGEV_THREAD) */
           void  *sigev_notify_attributes;
                                 /* Attributes for notification thread
                                    (SIGEV_THREAD) */
           pid_t  sigev_notify_thread_id;
                                 /* ID of thread to signal
                                    (SIGEV_THREAD_ID); Linux-specific */
       };
#endif


typedef unsigned int boolean;

struct sigevent sev;

typedef enum
{
	T1_TIMER = 0,
	T2_TIMER
}timer_type;

typedef struct
{
	timer_t timer_id;
	boolean is_running;
}timer_data;

typedef struct
{
	int important_data;
}timer_t1_data;

/* you can use this when there are many timers.*/
typedef struct {
	timer_data tdata;
	timer_type type;
	void* user_data;
}handler_data;

void timer_start(timer_data* tdata, timer_type ttype)
{
	struct itimerspec its;

	switch(ttype) {
		case T1_TIMER:
			/* Timer will expire after 5 seconds*/
			printf("\n this is T1 timer");
			its.it_value.tv_sec = 5;
			its.it_value.tv_nsec = 0;
			its.it_interval.tv_sec = 5;
			its.it_interval.tv_nsec =0;
		break;

		case T2_TIMER:
		break;
	}

	tdata->is_running = 1;
	if (-1 == timer_settime(tdata->timer_id, 0, &its, NULL)) {
		printf("error: settime");
		return;
	}
}

/* Will be called once the timer is expired. */
void timer_cb(int sig, siginfo_t* si, void* uc)
{
	handler_data* hdata;
	hdata = (handler_data *)si->si_value.sival_ptr;

	switch (hdata->type) {
		case T1_TIMER:
		/* Printf should not be called from the timer callback. check man 7 signal.*/
		printf("\n T1 timer expired");
		timer_t1_data* t1_data = hdata->user_data;
		printf("\n t1 data is %d", t1_data->important_data);
		break;
	}
	hdata->tdata.is_running = 0;
}

int main()
{
	struct sigaction sa;
	timer_t1_data t1_data;

	int timer_value;

	handler_data hdata;
	/* important data that is needed once the timer is expired. Since any timer can expire at any time,
		we may need some information regarding which timer has expired, and based on that action needs to be taken.
		This can be done by filling sival_ptr in sigevent */
	t1_data.important_data = 555;

	/* Notify the process by sending signal. */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGUSR1;
	hdata.type = T1_TIMER;
	hdata.user_data = (void *)&hdata.tdata;
	sev.sigev_value.sival_ptr = (void *)&hdata;

	/* CLOCK_REALTIME: Jump forwards and backward as system time changes. system time is which shows on the screen.
	   CLOCK_MONOTONIC: Represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past.
	   It isn't affected by changes in the system time-of-day clock.*/
	if(-1 == timer_create(CLOCK_REALTIME, &sev, &hdata.tdata.timer_id)) {
		printf("\n timer create failed. errno:%d", errno);
		return 0;
	}

	/* When you specify SA_SIGINFO, you can use the you can specify the signal handling function for the signal number.
		Else sa_handler will be used as the signal handling function. With sa_handler you can use SIG_DFL or SIG_IGN (i.e. default action of the signal or ignore the signal)*/
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_cb;

	/* Clear all the bits in the signal set*/
	sigemptyset(&sa.sa_mask);
	/* This system call is used to change the action taken by a process on receipt of signal. */
	if (-1 == sigaction(SIGUSR1, &sa, NULL)) {
		printf("error in sigaction %d", errno);
		return 0;
	}

	/*Now I will call the timer*/
	timer_start(&hdata.tdata, T1_TIMER);

	/*This is added becasue main function should not exit before timer expires.*/
	while(1);

	return 0;
}
