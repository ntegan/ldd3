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
      .p_process_infos = NULL,
  };
  int result = ioctl(dev_fd, KMOD_IOD, &kpr);
  if (result) return result;
  printf("got num_procs: %ld\n", kpr.num_process_infos_requested);

  kpr.num_process_infos_requested *= 2;
  unsigned long num_procs = kpr.num_process_infos_requested;
  kpr.p_process_infos =
      (kmod_process_info *)malloc(num_procs * sizeof(kmod_process_info));
  memset((void *)kpr.p_process_infos, 0, num_procs * sizeof(kmod_process_info));
  kpr.num_process_infos_fulfilled = 0;

  result = ioctl(dev_fd, KMOD_IOD, &kpr);
  if (result) return result;
  printf("got num_procs fulfilled: %ld\n", kpr.num_process_infos_fulfilled);

  for (int i = 0; i < kpr.num_process_infos_fulfilled; i++) {
    printf("pid %ld, mm 0x%016lX, name %s\n", kpr.p_process_infos[i].pid,
           kpr.p_process_infos[i].p_mm, kpr.p_process_infos[i].comm);
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
