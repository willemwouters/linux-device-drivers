#include "signal_misc.h"

#include <asm/siginfo.h>	//siginfo
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

int send_signal(int myPid) {
	/* send the signal */
	struct siginfo info;
	int ret;
	struct task_struct *t;
	struct pid *pid_struct;
	pid_t pid;
	memset(&info, 0, sizeof(struct siginfo));
	info.si_signo = SIG_TEST;
	info.si_code = SI_QUEUE;	// this is bit of a trickery: SI_QUEUE is normally used by sigqueue from user space,
					// and kernel space should use SI_KERNEL. But if SI_KERNEL is used the real_time data 
					// is not delivered to the user space signal handler function. 
	info.si_int = 1;  		//real time signals may have 32 bits of data.

	rcu_read_lock();

	pid = myPid; //integer value of pid
	pid_struct = find_get_pid(pid); //function to find the pid_struct
	t = pid_task(pid_struct,PIDTYPE_PID); //find the task_struct
	if(t == NULL){
		printk("no such pid\n");
		rcu_read_unlock();
		return -ENODEV;
	}
	rcu_read_unlock();
	ret = send_sig_info(SIG_TEST, &info, t);    //send the signal
	if (ret < 0) {
		printk("error sending signal\n");
		return ret;
	}
	return 0;
}
