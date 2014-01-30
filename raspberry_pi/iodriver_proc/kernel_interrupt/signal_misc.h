#ifndef _SIGNAL_MISC_H
#define _SIGNAL_MISC_H

#define SIG_TEST 44	// we choose 44 as our signal number (real-time signals are in the range of 33 to 64)

int send_signal(int myPid);

#endif //_SIGNAL_MISC_H
