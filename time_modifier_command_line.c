#define _DEFAULT_SOURCE

#include<errno.h>
#include<limits.h>
#include<linux/limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<time.h>
#include<unistd.h>

#define IOCONTROL_CMD _IOW(0xCD, 0x19, char*) 	// Allows Linux system call to access underlying device parameters of files

struct path_data 
{
	char* path;
	size_t size;
	struct timespec time;
	char* command;
}; 

int main(int argc, char **argv)
{
	int file = open("/dev/timem", O_RDWR);	// Open driver file
	
	// Check for error from opening file
	if(file < 0)
	{
		fprintf(stderr, "Could not open driver's device\n");
		return 1;
	}
	
	/*
	Four command line args:
		1. Program path (i.e. the path to ./changer)
		2. Path of item we wish to modify
		3. What trait to modify (LM, DA, DC) 
		4. Time
	*/

	// Check for correct number of arguments
	if(argc != 4) 
	{	
		fprintf(stderr, "Invalid number of arguments! \n");
		goto failure;	// Closes file and returns 1
	}

	// Begin parsing arguments
	char validPath[PATH_MAX];
	struct tm tinfo;
	struct path_data data;
	
	// Check that the file path is valid
	if(realpath(argv[1], validPath) != validPath)
	{
		fprintf(stderr, "Invalid path: '%s'\n", argv[1]);
		goto failure;	// Closes file and returns 1
	}

	data.path = validPath;
	data.size = strlen(validPath) + 1;

	// Get the third parameter and transfer its data to a time struct
	sscanf(argv[3], "%u.%u.%u-%u:%u:%u:%ld", &tinfo.tm_mday, &tinfo.tm_mon,
			&tinfo.tm_year, &tinfo.tm_hour, &tinfo.tm_min, &tinfo.tm_sec,
			&data.time.tv_nsec);
	tinfo.tm_year -= 1900; //relative time
	tinfo.tm_mon -= 1;
	data.time.tv_sec = mktime(&tinfo);
	if(data.time.tv_sec < 0 || data.time.tv_nsec < 0 || data.time.tv_nsec >= 100000000L){
		fprintf(stderr, "Invalid time: '%s'\n", argv[3]);
		goto failure;
	}


	if(strcmp(argv[2], "LM") == 0) 
	{
		data.command = "LM";
		printf("Processing...\n");
		
		/* 
		ioctl() - System call that manipulates the underlying 
		device parameters of files.
		Check that the system call modified the file time
		*/
		if(ioctl(file, IOCONTROL_CMD, &data) < 0)
		{
			fprintf(stderr, "Failed to change time of '%s': %s\n", argv[1], strerror(errno));
		}
	
		printf("Changed 'Last Modified' time of '%s' to %s\n", argv[1], asctime(&tinfo));

	} 
	else if (strcmp(argv[2], "DA") == 0) 
	{
		data.command = "DA";
		printf("Processing...\n");
		
		/* 
		ioctl() - System call that manipulates the underlying 
		device parameters of files.
		Check that the system call modified the file time
		*/

		if(ioctl(file, IOCONTROL_CMD, &data) < 0)
		{
			fprintf(stderr, "Failed to change time of '%s': %s\n", argv[1], strerror(errno));
		}
	
		printf("Changed 'Date Accessed' time of '%s' to %s\n", argv[1], asctime(&tinfo));
	} 
	else if (strcmp(argv[2], "DC") == 0) 
	{
		data.command = "DC";
		printf("Processing...\n");
		
		/* 
		ioctl() - System call that manipulates the underlying 
		device parameters of files.
		Check that the system call modified the file time
		*/

		if(ioctl(file, IOCONTROL_CMD, &data) < 0)
		{
			fprintf(stderr, "Failed to change time of '%s': %s\n", argv[1], strerror(errno));
		}
	
		printf("Changed 'Date Changed' time of '%s' to %s\n", argv[1], asctime(&tinfo));
	} 
	else 
	{
		fprintf(stderr,"%s is not a valid operation!\nPlease enter LM or DA or DC.\n", argv[2]);
		goto failure;
	}
	
	close(file);
	return 0;
	
	// Close file and return error if failure
failure:
	close(file);
	return 1;
}