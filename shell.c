/*
	Author: 		 Ian Isely 
	Date: 		 3/25/20
	Description: This program is a simple shell with support for a list of commands
					 located under help. The highlights of this shell are the set_scheduling
					 and exec commands. The exec command will execute the program p compiled
				    from the file p-shell.c which just prints a given amount of times. The
					 set_scheduling command allows the user to change the scheduling policy
					 of processes to either First Come First Serve (FCFS), Round Robin (RR), 
					 Multi-Level Feedback Queue (MFQ), or Preemptive Shortest Job First (SJF).

	Contributers: Dr. Iraklis Anagnostopoulos (Provided everything but: set_scheduling
					  command, MFQ, RR, SJF, and anything associated with such policies)
*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <sys/resource.h>
#include "queue.h"

//Global Variables
int done = 0;
int fg_pid=0;
int fg_suspended=0;
int run=1;
int qt, num_queues, argnum;
int p_count = 0;
char policy[5];
struct queue pid_list;


/*
	Function:    help()
	Arguments:   NONE
	Description: This function displays the list of supported functions by this shell
*/
void help() 
{
	printf("This is manual page\n");
	printf("This shell supports the following commands:\n");
	printf("\tver\n\texec\n\tps\n\tkill\n\thelp\n\texit\n\tset_scheduling\n");
	printf("For more details please type 'help command'\n");
}


/*
	Function:    helpcmd(char*)
	Arguments:   char* command
	Description: This function provides additional information about specified commands
*/
void helpcmd(char *cmd) 
{
	printf("This is manual page\n\n");
	if (strcmp(cmd,"ver")==0)
	{
		printf("\nver:\tShows details about the shell version\n");
	}
	else if (strcmp(cmd,"exec")==0) 
	{
		printf("\nexec p1(n1,qt1) p2(n2,qt2) ...:\nExecutes the programs p1, p2 ... \nEach program types a message for n times and it is given a time quantum of qt msec.\n");
		printf("If parameter (&) is given the program will be executed in the background\n");
	}
	else if (strcmp(cmd,"ps")==0)
	{
		printf("\nps:\tShows the living process with the given pid\n");
	}
	else if (strcmp(cmd,"kill")==0)
	{
		printf("\nkill pid:\tEnds the process with the given pid\n");
	}
	else if (strcmp(cmd,"help")==0)
	{
		printf("\nhelp:\tYou should know this command by now\n");
	}
	else if (strcmp(cmd,"exit")==0)
	{
		printf("\nexit:\tEnds the experience of working in the new shell\n");
	}
   else if (strcmp(cmd,"set_scheduling")==0)
   {
   	printf("\nset_scheduling policy:\tWill change how the scheduler operates with process execution\n");
      printf("policy: FCFS, SJF, MFQ, or RR\n");
   }
	else 
		printf("\nNo Such command. Type help to see a list of commands\n");
}


/*
	Function:    ver()
	Arguments:   NONE
	Description: This function just provides version info of the shell
*/
void ver() 
{
	printf("\nNew Shell (3/25/2020) working properly!!\n");
}


/*
	Function:    ps()
	Arguments:   NONE
	Description: This function will display the pid and name all living processes to the user.
					 This function will not work as intended for MFQ scheduling
*/
void ps() 
{
	struct node *p;
	printf("\nNEW SHELL presents the following living processes\n");
	printf("\tPID\tNAME\n");
	for (p=pid_list.head; p!=NULL; p=p->next)
	{
		printf("\t%d\t%s\n",p->pid,p->name);
	}
}


/*
	Function:	 mykill(int)
	Arguments:   int pid
	Description: This function will terminate the process with the given pid
*/
void mykill(int pid) 
{
	kill(pid,SIGTERM);
	printf("You have just killed process %d\n",pid);
}


/*
	Function:	 exec(char*)
	Arguments:   char* input
	Description: This function will execute the given program by forking a child and
					 binding it to the executable with execv(). This function supports
					 FCFS, RR, SJF, and MFQ scheduling of its active processes. 
	
	ps: Not to be confused with the exec command of the shell which takes as inputs
		 the number of prints and a time quantum for the p-shell program
*/
void exec(char *input) 
{
	int i, t, c_pid;
   char c_name[10];
	char *args[10];
	char *temp;
	struct queue *qp;

	for (i = 0; i < 10; i++)
	{
		args[i]=(char *)malloc(10*sizeof(char));
	}

	strcpy(args[0],strtok(input,"(,"));
	for (i=1; (temp=strtok(NULL,",)"))!=NULL; i++) 
		strcpy(args[i],temp);
	printf("\n");
	if (strcmp(args[i-1],"&")==0)
		args[i-1]=NULL;
	else
		args[i]=NULL;


   if(strcmp(policy,"FCFS") == 0) /** First Come First Serve Scheduling **/
   {
	   if ((t=fork())==0)
	   {
			execv(args[0],args);
	   }
	   enqueue(t,args[0],&pid_list);

      usleep(10000); //safe delay

	   if (args[i-1]!=NULL) //If & argument not given
	   {
         fg_pid=t;
			kill(t,SIGCONT);
			while(fg_pid != 0 && fg_suspended != 1)
         {
				pause();
			}
	   }
      else
      	kill(t,SIGCONT);
	}
   else if(strcmp(policy,"RR") == 0) /** Round Robin Scheduling **/
  	{
	   if((t=fork())==0)
		{
			execv(args[0],args);
		}
		else
		{
			enqueue(t,args[0],&pid_list);
			p_count++;
			if(p_count != argnum) //create all processes before executing any
			{
			   return;
			}
		}
      usleep(10000); //safe delay

		while(pid_list.head != pid_list.tail)
		{
			kill(pid_list.head->next->pid, SIGCONT);
         usleep(qt);

			if(!done)
      	{
         	kill(pid_list.head->next->pid, SIGTSTP);
         	usleep(1000); //safe delay;

         	c_pid = pid_list.head->next->pid;
				strcpy(c_name, pid_list.head->next->name);
         	dequeue(&pid_list);
         	enqueue(c_pid, c_name, &pid_list);
      	}
      	else
      	{
         	done = 0;
      	}
			usleep(1000); //safe delay
		}
    	p_count = 0;
		dequeue(&pid_list);
		enqueue(getppid(),"NEW SHELL",&pid_list);
   }
  	else if(strcmp(policy,"MFQ") == 0) /** Multi-Level Feedback Queue Scheduling **/
   {
		int dead;

		if((t=fork())==0)
		{
			execv(args[0],args);
		}
		else
		{
			enqueue(t, args[0], pid_list.next);
			p_count++;
			if(p_count != argnum) //create all processes before executing
			{
			   return;
			}
		}
      usleep(1000); //safe delay

		for(qp = pid_list.next; qp->head != NULL; qp = qp->next)
		{
			while(qp->head != NULL)
			{
				kill(qp->head->pid, SIGCONT);
         	usleep(qt);

         	if(!done)
         	{
            	kill(qp->head->pid, SIGTSTP);
            	usleep(1000); //safe delay;

            	c_pid = qp->head->pid;
					strcpy(c_name, qp->head->name);
            	dequeue(qp);
					if(qp->next != NULL)
            		enqueue(c_pid, c_name, qp->next); //Move child to low prio queue
					else
						enqueue(c_pid, c_name, qp); //Move child to back of current queue
         	}
         	else
				{
            	done = 0;
					dead++;
				}
			}
			if(dead == p_count) break;
		}
		p_count = 0;
		usleep(1000); //safe delay
	}
   else if(strcmp(policy,"SJF") == 0) /** Shortest Job First Scheduling **/
   {
	   if ((t=fork())==0)
	   {
			execv(args[0],args);
	   }
	   sjf_enqueue(t, atoi(args[1]), args[0], &pid_list.head->next);
		p_count++;
      usleep(10000); //safe delay
		if(pid_list.head->next->pid == t && p_count > 1)
		{
			kill(pid_list.head->next->next->pid, SIGTSTP);
			usleep(1000); //safe delay
		} 

	   if (args[i-1]!=NULL) //If & argument not given
	   {
         fg_pid=t;
			kill(t,SIGCONT);
			while(fg_pid != 0 && fg_suspended != 1)
         {
				pause();
			}
			p_count = 0;
	   }
      else
		{
			if(pid_list.head->next->pid == t)
      		kill(t,SIGCONT);
		}
	}

}


/*
	Function:
	Arguments:
	Description: This function will close the shell unless other processes
				 	 are still living, for which it will ask the user to kill
					 them or not.
*/
void myexit() 
{
	char yesno;
	if (pid_list.head==pid_list.tail)
	{
		run=0;
	}
	else {
		printf("There are still living processes Do you want to kill them? (y/n): ");
		yesno=getchar();
		if (yesno == 'y')
		{
			while(pid_list.head!=pid_list.tail)
			{
				mykill(pid_list.tail->pid);
				usleep(10000);
			}
			run=0;
		}
	}
}


/*
	Function:	 childdead(int)
	Arguments:   int signum
	Description: This is a signal handler for whenever the shell
					 recieves SIGCHLD and will deallocate the memory
					 it used and dequeue it from the active pid list.
*/
void childdead(int signum)
{
	int dead_pid, status, i;
	struct queue *qp;

	done = 1;	
	dead_pid=wait(&status);
	printf("The child %d is dead\n",dead_pid);
	
	if(strcmp(policy,"MFQ") == 0)
	{
		qp = pid_list.next;
		for(i=0;i<num_queues;i++)
		{
			delete(qp, dead_pid);
			qp = qp->next;
		}
	}
	else
		delete(&pid_list, dead_pid);

	if(strcmp(policy,"SJF") == 0 && pid_list.head->next != NULL)
		kill(pid_list.head->next->pid, SIGCONT);

	printf("\n");
	if (dead_pid==fg_pid)
	{
		fg_pid=0;
	}
}


/*
	Function:	 susp(int)
	Arguments:   int signum
	Description: This signal handler is triggered whenever the shell
					 recieves SIGTSTP and will cause all active processes
				    to suspend.
*/
void susp (int signum) 
{
	fg_suspended=1;
	printf("All processes supspended\n");
}


/*
	Function:	 cont(int)
	Arguments:   int signum
	Description: This signal handler is triggered whenever the shell
					 recieves SIGQUIT and will wake all dormant processes.
*/
void cont (int signum) 
{
	fg_suspended=0;
	printf("Waking all processes...\n");
	while (fg_pid!=0 && fg_suspended!=1)
		pause();
}


/*
	Function:    my_policy(char*)
	Arguments:   char* policy
	Description: This function will change the scheduling policy of the shell
					 according to what is chosen. 
*/
void my_policy(char * pop)
{
	num_queues = 0;

   if(pid_list.head == pid_list.tail) //If no processes running but shell
   {
      if(strcmp(pop,"FCFS") == 0)
      {
         strcpy(policy, pop);
         printf("Scheduling policy set to First Come First Serve\n");
      }
      else if(strcmp(pop,"RR") == 0)
      {
         printf("Enter a time quantum(ms): ");
         scanf("%d", &qt);
         qt = qt*1000;
         strcpy(policy, pop);
         printf("Scheduling policy set to Round Robin\n");
      }
      else if(strcmp(pop,"MFQ") == 0)
      {
			int i;
         printf("Enter a time quantum(ms): ");
         scanf("%d", &qt);
         qt = qt*1000;
         while(num_queues <= 1 || num_queues >= 5) //2-4 queues
         {
            printf("Enter the number of queues(2-4): ");
            scanf("%d", &num_queues);
         }
         strcpy(policy,pop);

			//Create feedback queues
			struct queue * qp;
			qp = &pid_list;
			for(i=0; i<num_queues; i++)
			{
				qp->next = (struct queue *) malloc(sizeof(struct queue));
				if(qp->next == NULL)
   			{
      			printf("OUT OF MEMORY!!\n");
      			exit(1);
   			}
				qp = qp->next;
			}
			qp->next = NULL;
         printf("Scheduling policy set to Multi-Level Feedback Queue\n");
      }
      else if(strcmp(pop,"SJF") == 0)
      {
         strcpy(policy, pop);
         printf("Scheduling policy set to Preemptive Shortest Job First\n");
      }
      else
         printf("Unknown input argument, refer to help page\n");
   }
   else
      printf("Unable to change scheduling policy if processes are running\n");
}


/* MAIN */
int main(int argc, char const *argv[])
{
	char input[15][30];
	int i;

	pid_list.head=NULL;
	pid_list.tail=NULL;
	enqueue(getppid(),"NEW SHELL",&pid_list);

	signal(SIGCHLD,childdead);
	signal(SIGTSTP,susp);
	signal(SIGQUIT,cont);
	setpriority(PRIO_PROCESS,0,-20);

	strcpy(policy,"FCFS"); //Default scheduling policy

	ver();

	while (run)
	{
		printf("=>");
		for (argnum=0; (scanf("%s",&input[argnum]))==1;argnum++)
			if (getchar()=='\n') 
				break;

		if (strcmp(input[0],"ver")==0 && argnum==0) 
		  	ver();
		else if (strcmp(input[0],"help")==0 && argnum==0) 
			help();
		else if (strcmp(input[0],"help")==0 && argnum==1) 
			helpcmd(input[argnum]);
		else if (strcmp(input[0],"ps")==0 && argnum==0) 
			ps();
		else if (strcmp(input[0],"kill")==0 && argnum==1) 
			mykill(atoi(input[1]));
		else if (strcmp(input[0],"exec")==0 && argnum!=0)
		{
			for (i=1; i<=argnum; i++) 
				exec(input[i]);
			if(strcmp(policy,"SJF") == 0 && pid_list.head == pid_list.tail)
			{
				usleep(10000);
				p_count = 0;
			}
		}
      else if (strcmp(input[0],"set_scheduling")==0 && argnum == 1)
      	my_policy(input[1]);
		else if (strcmp(input[0],"exit")==0 && argnum==0) 
			myexit();
	   else 
			printf("No such command. Check help for help.\n");
	}

   return 0;
}
