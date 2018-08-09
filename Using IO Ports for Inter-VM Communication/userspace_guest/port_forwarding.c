#include <sys/io.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 0x0092 /* PS2 legacy I/O Port specific to x86*/

int main()
{
	int choice;
	printf("enter option :\n");
	printf("For Input press %d\n", 1);
	printf("For Output press %d\n", 2);	
	scanf("%d", &choice);
	
	if(ioperm(PORT,3,1))	
	{
		//perror("perm	ission could not be set");
		exit(1);
	}

	switch(choice)
	{
		case 1 :
		{
			while(1){
				//printf("Ready to receive value \n");
				printf("Received value in the port 0x0092 : %c \n", (65+inb(PORT)));
				/*for(int i=0; i<1000;i++){
					
				}*/
			}
			break;		
		}
		
		case 2 :
		{	int count=0;
			while(1){
				printf("Ready to send value in the port 0x0092: %c \n", (65+count));						
				outb((0+count),PORT);
				/*for(int i=0; i<30000;i++){
					
				}*/
				count++;
				if(count == 20)
					count=0;
			}	
			break;	
		}
		
	}
	exit(0);

	ioperm(PORT,3,0);	
}
