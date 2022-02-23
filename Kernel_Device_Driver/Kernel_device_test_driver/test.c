#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define NODE_NAME "/dev/gpio_Alive"
//#define NODE_NAME "/dev/gpio_Alive2"
//#define NODE_NAME "/dev/gpio_Alive4"
 
int main(int argc, char * argv[])
{
    int fd;
    char dev_name[60];
    for(int i = 0; i<= 1; i++)
    {
        sprintf(dev_name, "%s%d", NODE_NAME, (i+1)*2);
        fd = open(dev_name, O_RDWR);

        if(fd < 0) {
            printf("%s Device open error\n", dev_name);
            return -1;
        }

        read(fd, NULL, 0);
        write(fd, NULL, 0);
        ioctl(fd, 0, NULL);
        close(fd);
    }
}
