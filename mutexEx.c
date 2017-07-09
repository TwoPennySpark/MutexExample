#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define PTHREAD_NUM 10
#define BUFFER_SIZE 128

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void dieWithError(char *msg)
{
	printf("[-]ERROR:%s\n", msg);
	exit(0);	
}

void* writer(void *arg)
{
	int fd, string_num = 1;
	char buffer[BUFFER_SIZE];
	time_t rawtime; 
	struct tm *timeinfo;

	while(1)
	{
		pthread_mutex_lock(&mutex);
		time(&rawtime);   
		timeinfo = localtime(&rawtime);
		if ((fd = open("file1", O_CREAT|O_WRONLY|O_APPEND, 0744)) < 0)
			dieWithError("Can't open the file\n");
		sprintf(buffer, "string number:%d %d:%d:%d\n", string_num, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		if (write(fd, buffer, strlen(buffer)) < 0)
			dieWithError("write() failed"); 
		string_num++;
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	pthread_exit(0);
}

void* reader(void *arg)
{
	int fd, buffer_read = 0, buffer_written = 0;
	char buffer[BUFFER_SIZE];
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if ((fd = open("file1", O_CREAT|O_RDONLY, 0744)) < 0)
			dieWithError("Can't open the file\n");
		printf("=============================================\n");
		printf("\tReader thread[%ld]\n", pthread_self());
		printf("=============================================\n");
		while ((buffer_read = read(fd, buffer, BUFFER_SIZE)) > 0)
		{
		    buffer_written = write (1, buffer, buffer_read);
		    if (buffer_written != buffer_read)
		        dieWithError("Write() failed\n");
		}	
		pthread_mutex_unlock(&mutex);
		sleep(5);
	}
	pthread_exit(0);   
}

int main(void)
{
	pthread_t read_tid[PTHREAD_NUM];
	pthread_t write_tid;
	int i = 0;

	if (pthread_create(&write_tid, NULL, writer, NULL) < 0)
		dieWithError("Can't create writer thread\n");
	for (i = 0; i < PTHREAD_NUM; i++)
		if (pthread_create(&read_tid[i], NULL, reader, NULL) < 0)
			dieWithError("Can't create reader thread\n");

	if (pthread_join(write_tid, NULL) < 0)
		dieWithError("Join writer failed\n");
	for (i = 0; i < PTHREAD_NUM; i++)
		if (pthread_join(read_tid[i], NULL) < 0)
			dieWithError("Join reader failed\n");
	return 0;
}	 
