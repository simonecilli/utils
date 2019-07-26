/*
 * @brief GPIO usage example using GPIO Sysfs Interface for Userspace
 * 
 * THIS ABI IS DEPRECATED, THE ABI DOCUMENTATION HAS BEEN MOVED TO
 * Documentation/ABI/obsolete/sysfs-gpio AND NEW USERSPACE CONSUMERS
 * ARE SUPPOSED TO USE THE CHARACTER DEVICE ABI. THIS OLD SYSFS ABI WILL
 * NOT BE DEVELOPED (NO NEW FEATURES), IT WILL JUST BE MAINTAINED.
 * Refer to the examples in tools/gpio/* for an introduction to the new
 * character device ABI. Also see the userspace header in
 * include/uapi/linux/gpio.h
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// GPIO name could change in some systems
void getGPIOName(char *s, int gpio, const char* attribute)
{
	sprintf(s, "/sys/class/gpio/%d/%s", gpio, attribute);
}

int main(int argc, char **argv)
{
    int fd, gpio, val0, val, dir0, dir, n;
    char buf[128];

	// Handle parameters
	switch(argc)
	{
		case 4:
			gpio = atoi(argv[1]);
			dir  = atoi(argv[1]);
			n    = atoi(argv[1]);
			break;
		default:
			printf("%s <gpio> <dir> <n>", argv[0]);
			printf(" <gpio>: GPIO number\n");
			printf("  <dir>: 0->output, 1->input\n");
			printf("    <n>: Number of read/write\n");
			break;		
	}

	// Export
	fd = open("/sys/class/gpio/export", O_WRONLY);
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%d", gpio);
	write(fd, buf, strlen(buf));
	close(fd);
	
	// Direction - Read
	getGPIOName(buf, gpio, "direction");
	fd = open(buf, O_RDONLY);
	memset(buf, '\0', sizeof(buf));
	read(fd, buf, 1);
	dir0 = (buf[0] == 'i')?1:0; //TODO: Verify. Input should be "in" or "out"
    close(fd);
	
	// Value - Read
	getGPIOName(buf, gpio, "value");
	fd = open(buf, O_RDONLY);
	memset(buf, '\0', sizeof(buf));
	read(fd, buf, 1);
	val0 = atoi(buf);
	val = val0;
    close(fd);
	
	// Current status
	printf("GPIO TOOL\n");
	printf(" <gpio>: %d\n", gpio);
	printf("  <dir>: %d (current:%d)\n", dir, dir0);
	printf("  <val>: -  (current:%d)\n", val0);
	printf("    <n>: %d\n", n);
	
	// Direction - Write
	// reads as either "in" or "out". This value may normally be written.
	// Writing as "out" defaults to initializing the value as low. To ensure
	// glitch free operation, values "low" and "high" may be written to
	// configure the GPIO as an output with that initial value.
	// Note that this attribute *will not exist* if the kernel doesn't support
	// changing the direction of a GPIO, or it was exported by kernel code that
	// didn't explicitly allow userspace to reconfigure this GPIO's direction.
	getGPIOName(buf, gpio, "direction");
	fd = open(buf, O_WRONLY);
	if(!dir) // IN
	{
		if(dir0)
		{
			write(fd, "in", 2);
		}
	}
	else // OUT
	{
		if (val)
		{
			write(fd, "high", 4);
		}
		else
		{
			write(fd, "low", 3);
		}
	}
    close(fd);

	// Value - Loop of n Read/Write
	// Considert also edge, active_low, ...
	getGPIOName(buf, gpio, "value");
	fd = open(buf, (dir==0)?O_RDONLY:O_WRONLY);
	memset(buf, '\0', sizeof(buf));
	for(int i=0; i<n; i++)
	{
		if(!dir)
		{
			read(fd, buf, 1);
			printf("GPIO%d/value: %s", gpio, buf);
		}
		else
		{
			val = (val+1)%2;
			write(fd, (!val)?"0":"1", 1);
			printf("GPIO%d/value: %s", gpio, buf);
		}
		sleep(1);
	}
	close(fd);
	
	// Restore direction and value
	getGPIOName(buf, gpio, "direction");
	fd = open(buf, O_WRONLY);
	if(!dir0) // IN
	{
		write(fd, "in", 2);
	}
	else // OUT
	{
		if (val0)
		{
			write(fd, "high", 4);
		}
		else
		{
			write(fd, "low", 3);
		}
	}
    close(fd);
	
	// Unexport
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	sprintf(buf, "%d", gpio);
	write(fd, buf, strlen(buf));
	close(fd);
	
	return 0;
} 