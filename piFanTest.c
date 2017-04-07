#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_LENGTH 256
static char flag[BUFFER_LENGTH] = "HI";

const char *DESCRIPTION = "FanPi, by jean.\n To turn the fan on manually, type \"sudo ./fanpi on\" \n To turn the fan off manually, type \"sudo ./fanpi off\" \n To let the fan run on auto, type \"sudo ./fanpi auto\" \n To check fanPi's status, type \"sudo ./fanpi status\"\n ";

void autoCntl(void);
int initmod();

int initMod(){

	int kFd = open("fanPi.ko", O_RDONLY);
    		struct stat st;
    		fstat(kFd, &st);
    		size_t image_size = st.st_size;
    		void *image = malloc(image_size);
    		read(kFd, image, image_size);
    		close(kFd);
    
			if (init_module(image, image_size, "") != 0) {
        			perror("init_module");
    			}
    free(image);
return 0;

}

int fanCntl(){
   	int ret, fd, cntlErr;
		 
	fd = open("/dev/fanPi", O_RDWR); 
	
	if (fd < 0) {
		perror("open failed");
		return errno;
   	}
	
	printf("fanPi set to [%s].\n", flag); 
	ret = write(fd, flag, strlen(flag));
	
	if (ret < 0) {
      		perror("write failed");
      		return errno;
   	} 
   	
	if (strcmp(flag, "ON") == 0){ printf("Fan turned on successfully!\n");} 
	else if (strcmp(flag, "OFF") == 0){ printf("Fan turned off successfully!\n");}

	return close(fd); 	
}

void status(){
	int t,statusRet, sFd, statusErr, tRet, tFd, tErr;
	static char status[BUFFER_LENGTH];
	char temp[BUFFER_LENGTH];	
	
	sFd = open("/dev/fanPi", O_RDONLY);
	if (sFd < 0) perror("open failed");
	statusRet = read(sFd, status, BUFFER_LENGTH);
	if (statusRet < 0) perror("read failed");
	close(sFd);
	
	tFd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
	tRet = read(tFd, temp, BUFFER_LENGTH);
	if (tRet < 0) perror("read failed");
	t = atoi(temp);
	t /= 1000;
	close(tFd);
	
	printf("\t\t***STATUS***\nFan is currently set to %s \n", status); 
	printf("CPU temperature is %d degrees c\n", t);

}

void autoCntl(){
	int tempret, tempfd, temperr;
	char temp[BUFFER_LENGTH];
	int t=0;

	while (1){
		tempfd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);	
		tempret = read(tempfd, temp, BUFFER_LENGTH);
	
		if (tempret < 0) perror("read failed");
	
		t = atoi(temp);
		t /= 1000;
		close(tempfd);	
		//REMOVE COMMENT TO DEBUG
		//printf("Current CPU tempurature= %d degrees\n", t);
		if ( t >= 40 ){

			if( strcmp(flag,"ON") != 0 ) {
				printf("STARTING FAN");
				strcpy(flag, "ON");	
				temperr = fanCntl();
			}

		}	 
	
		if ( (t < 30) && strcmp(flag, "ON") == 0 ){
			strcpy(flag,"OFF");
			temperr = fanCntl();
		}	

		sleep(5);
	} 
}

int main(int argc, char* argv[]) {
	FILE *fd = popen("lsmod | grep fanPi", "r");

  	char buf[16];



	int ierr;	
      
  	if (!(fread (buf, 1, sizeof (buf), fd) > 0)){
		ierr = initMod(); 	
 		if (ierr > 0) return 1;   	
	}

	int err;
	if (argc < 2){
		printf("%s", DESCRIPTION);
		
		return 1;
	}
	
	if (geteuid() != 0) { fprintf(stderr, "Error: fanpi needs to be run as root, sorry.\n"); 
				return 1;}

	if (strcmp(argv[1], "on") == 0) { 
		strcpy(flag, "ON"); 
		err = fanCntl();
	} else if (strcmp(argv[1], "off") == 0) { 
		strcpy(flag, "OFF"); 
		err = fanCntl();
	} else if (strcmp(argv[1], "auto") == 0) {
		int fd = open("/dev/fanPi", O_RDWR);
		int ret = write(fd, "AUTO", strlen("AUTO"));
		if (ret < 0) {perror("write failed");}
		close(fd); 
		int c_pid;
		printf("Fan will now monitor CPU tempurature!\n"); 
		c_pid = fork();
			
		if (c_pid == 0) { // child process
			autoCntl();
		}

	} else if (strcmp(argv[1], "status") == 0) {
		status();
		return 0;
	} else {
		printf("Sorry, command not found.\n %s", DESCRIPTION);
		
		return 1;
	}
	

}

