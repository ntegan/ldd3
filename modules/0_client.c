#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "0.h"

int open_my_device() {
  int fd = open("/dev/egan", O_RDWR);
  if (fd == -1) return 0;
  return fd;
}
int do_the_ioctl(int dev_fd) {
  kmod_process_request kpr = {
      .num_process_infos_requested = 0,
      .p_process_infos = 0xBEEF,
  };
  kpr.p_process_infos = malloc(1024 * sizeof(char));
  memset((void *)kpr.p_process_infos, 0, 1024 * sizeof(char));
  int result = ioctl(dev_fd, KMOD_IOD, &kpr);
  if (result) return result;
  if (strlen((char *)kpr.p_process_infos) > 1) {
    printf("it was omore than 1\n");
    printf("got string: %s\n", (char *)kpr.p_process_infos);
  }

  free(kpr.p_process_infos);
  return 0;
}

int main(int argc, char **argv) {
  int dev_fd = open_my_device();
  printf("Got fd: %d\n", dev_fd);

  if (!do_the_ioctl(dev_fd)) {
    printf("Successful ioctl\n");
  }

  if (!close(dev_fd)) {
    printf("Successful close!\n");
  }

  return 0;
}
