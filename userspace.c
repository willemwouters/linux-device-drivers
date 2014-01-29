#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 

#define SIG_TEST 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 


int open_port(const char * port, int & portnum)
{
    portnum = open(port, O_RDONLY | O_NOCTTY);
    if (portnum == -1)
    {
	return -1;
    }
    return 0;
}

int read_port(char * buffer, int port)
{
    int n = read(port, buffer, sizeof(buffer));
    if (n < 0) {
	return -1;
    }
    return 0;
}

void receiveData(int n, siginfo_t *info, void *unused) {
	char buffer[25];
	int port = 0;
	int res;
	res = open_port("/dev/simple4", port);
	if(res != 0) {
		perror("open_port: Unable to open port - ");
		return;
	}
	
	res = read_port(buffer, port);
	if(res != 0) {
		perror("read_port: Unable to read port - ");
	}
	printf("%s \n", buffer);
}

int main ( int argc, char **argv )
{
	int configfd;
	char buf[10];
	/* setup the signal handler for SIG_TEST 
 	 * SA_SIGINFO -> we want the signal handler function with 3 arguments
 	 */
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIG_TEST, &sig, NULL);
	/* kernel needs to know our pid to be able to send us a signal ->
 	 * we use debugfs for this -> do not forget to mount the debugfs!
 	 */
	configfd = open("/proc/iodriverpid", O_WRONLY);
	if(configfd < 0) {
		perror("open");
		return -1;
	}
	sprintf(buf, "%i", getpid());
	printf("pid: %d \n", getpid());
	if (write(configfd, buf, strlen(buf) + 1) < 0) {
		perror("fwrite"); 
		return -1;
	}

	while(1) {
		sleep(1);
	}
	
	return 0;
}



