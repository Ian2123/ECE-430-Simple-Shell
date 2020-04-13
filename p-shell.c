#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/resource.h>

void cont (int sig_num) {
   signal(SIGCONT,cont);
   printf("The child %d starts\n", getpid());
}

void stop (int sig_num) {
	signal(SIGTSTP,stop);
   printf("The child %d stops\n", getpid());
   pause();
}


int main (char *argc[], char *argv[]) {

	int i,num,sltime;

	signal(SIGCONT,cont);
	signal(SIGTSTP,stop);
	
	num =atoi(argv[1]);
	sltime = 1000*atoi(argv[2]);

   pause();
	
	for (i=1; i<=num; i++){
		printf("This is program %s (%d) and it prints for the %d time of %d...\n",argv[0],getpid(),i,num);
		usleep(sltime);
	}
        return 0;
}
