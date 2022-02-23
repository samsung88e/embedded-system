/*
 * test_program.c
 * by hyeyoo
 *
 * description:
 * this program tests device driver (made by driver.c)
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#define IOCTL_PRINT 1

int   main(void) {
  int fd;
  char buf[1000];
  int read_ret, write_ret;

  fd = open("/dev/example", O_RDWR);
  if (fd < 0) {
    printf("failed opening device: %s\n", strerror(errno));
    return 0;
  }

  write_ret = write(fd, "hello", 5);
  read_ret = read(fd, buf, 5);
  printf("fd = %d, ret write = %d, ret read = %d\n", fd, write_ret, read_ret);
  printf("content = %s\n", buf);

  ioctl(fd, IOCTL_PRINT, NULL);
  close(fd);
}