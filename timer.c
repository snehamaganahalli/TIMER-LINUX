#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<errno.h>
#include <unistd.h>

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

	if (-1 == timer_settime(tdata->timer_id, 0, &its, NULL)) {
		printf("error: settime");
		return;
	}
}

/* Will be called once the timer is expired*/
void timer_cb(int sig, siginfo_t* si, void* uc)
{
	handler_data* hdata;
	hdata = (handler_data *)si->si_value.sival_ptr;

	switch (hdata->type) {
		case T1_TIMER:
		printf("\n T1 timer expired");
		timer_t1_data* t1_data = hdata->user_data;
		printf("\n t1 data is %d", t1_data->important_data);
		break;
	}
	/* Printf should not be called from the timer callback. check man 7 signal.*/

}

int main()
{

	struct sigaction sa;
	timer_data tdata = {0};

	int timer_value;

	timer_t1_data t1_data;
	handler_data hdata;
	/* important data that is needed once the timer is expired. Since any timer can expire at any time,
		we may need some information regarding which timer has expired, and based on that action needs to be taken.
		This can be done by filling sival_ptr in sigevent*/
	t1_data.important_data = 555;

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGUSR1;
	hdata.type = T1_TIMER;
	hdata.user_data = (void *)&t1_data;
	sev.sigev_value.sival_ptr = (void *)&hdata;

	if(-1 == timer_create(CLOCK_REALTIME, &sev, &tdata.timer_id)) {
		printf("\n timer create failed. errno:%d", errno);
		return 0;
	}

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_cb;

	sigemptyset(&sa.sa_mask);
	if (-1 == sigaction(SIGUSR1, &sa, NULL)) {
		printf("error in sigaction %d", errno);
		return 0;
	}

	/*Now I will call the timer*/
	timer_start(&tdata, T1_TIMER);

	/*This is added becasue main function should not exit before timer expires.*/
	while(1);

	return 0;
}
