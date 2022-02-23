#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
 
// 생성된 장치파일 위치 및 이름
#define NODE_NAME "/dev/drv_hello"
 
int main(int argc, char * argv[])
{
  // 파일 열기
  // 설정한 open 함수가 불린다.
  int fd = open(NODE_NAME, O_RDWR);
  if(fd < 0)
    {
      printf("%s open error... \n", NODE_NAME);
      return -1;
    }
  // 설정한 read 함수가 불린다.
  read(fd, NULL, 0);
  // 설정한 write 함수가 불린다.
  write(fd, NULL, 0);
  // 설정한 ioctl 함수가 불린다.
  ioctl(fd, 0, 0);
  // 설정한 release 함수가 불린다.
  close(fd);
}
