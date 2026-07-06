#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG_MESSAGES
#define DPRINT(...) printf(__VA_ARGS__);
#else
#define DPRINT(...)
#endif /* ifdef DEBUG_MESSAGES                                                 \
#define DPRINT(x, args) */

extern char *cpuTempPath;

int waitDevice(char *name) {
  // while (1) {
  int device = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
  printf("trying to get device...\n");
  if (device > 0) {
    printf("got device descriptor\n");
    struct termios t;
    tcgetattr(device, &t);
    cfmakeraw(&t);
    t.c_cflag |= (CLOCAL | CREAD);
    t.c_cflag &= ~PARENB;
    t.c_cflag &= ~CSTOPB;
    t.c_cflag &= ~CSIZE;
    t.c_cflag |= CS8;
    cfsetispeed(&t, B115200);
    cfsetospeed(&t, B115200);
    tcsetattr(device, TCSANOW, &t);
    return device;
  } else {
    perror("failed opening device descriptor");
    exit(1);
  }
  // sleep(1);
  //}
}
int getDeviceOutput(int device, char *buf, int len, int maxtry) {
  memset(buf, '\0', len);
  int try = 0;
  while (1) {
    char c;
    if (read(device, &c, 1) == -1) {
      perror("device io error");
      return -1;
    }
    if (c == '!')
      break;
    if (c != 0) {
#ifdef DEBUG_MESSAGES
      printf("%d\t%c\n", c, c);
#endif
      char cs[2];
      cs[1] = '\0';
      cs[0] = c;
      strcat(buf, cs);
    } else {
      try++;
    }
    if (try > maxtry)
      return 0;
    usleep(1000);
  }
  return 1;
}
const char DEVICE_WRITE_DATA_ERR_MSG[] =
    "error occure while writing data into device\n";
const int MAX_TRIES = 1024;
int makeConnection(int device) {
  if (write(device, "init", 4) != -1) {
    char buf[5];
    while (getDeviceOutput(device, buf, sizeof(buf), MAX_TRIES) == -1)
      usleep(1000);
    if (strcmp(buf, "conn") == 0)
      return 1;
  }
  return 0;
}

enum DeviceMod { INITIALISATION, DATE, ATMP };
enum DeviceMod mode = INITIALISATION;
void acceptDeviceMod(int device) {
  char buf[5];
  while (getDeviceOutput(device, buf, sizeof(buf), MAX_TRIES) != 1)
    usleep(1000);
  if (strcmp(buf, "date") == 0) {
    mode = DATE;
    DPRINT("accepted DATE mode from device\n");
    if (write(device, "done", 4) == -1)
      perror(DEVICE_WRITE_DATA_ERR_MSG);

  } else if (strcmp(buf, "atmp") == 0) {
    mode = ATMP;
    DPRINT("accepted ATMP mode from device\n");
    if (write(device, "done", 4) == -1)
      perror(DEVICE_WRITE_DATA_ERR_MSG);

  } else {
    DPRINT("got unrecognized command from device\n");
    if (write(device, "err!", 4) == -1)
      perror(DEVICE_WRITE_DATA_ERR_MSG);
  }
}
char *findCPUTempFile() {
  FILE *stream = popen("find /sys/devices/ -name 'temp1_input'", "r");
  char *path = malloc(1024 * sizeof(char));
  char *result = fgets(path, 1024, stream);
  fclose(stream);
  if (result != NULL) {
    path[strlen(path) - 1] = '\0';
    return path;
  } else {
    free(path);
    return NULL;
  }
}
int getCPUTemp(char *path) {
  if (path != NULL) {
    FILE *termstream = fopen(path, "r");
    if (termstream == NULL) {
      perror("failed reading cpu temp");
      return -1;
    }
    char temp[64];
    char *result = fgets(temp, sizeof(temp), termstream);
    fclose(termstream);
    if (result != NULL) {
      int temp = atoi(result);
      return temp / 1000;
    } else
      return -1;
  } else {
    return -1;
  }
}
void listenDeviceCommand(int device) {
  char buf[5];
  while (getDeviceOutput(device, buf, sizeof(buf), MAX_TRIES) != 1)
    usleep(1000);
  if (strcmp(buf, "chmd") == 0) {
    DPRINT("got CHMD command from device, ready to change the mode\n");
    DPRINT("send RED command to device\n");
    write(device, "read", 4);
    DPRINT("accepting new mode\n")
    acceptDeviceMod(device);
  } else if (strcmp(buf, "keep") == 0) {
    switch (mode) {
    case DATE: {
      time_t date = time(NULL);
      struct tm *t = localtime(&date);
      int test = t->tm_hour * 100 + t->tm_min;
      DPRINT("DATE: sending date\n");
      write(device, &test, 4);
      break;
    }
    case ATMP: {
      int temp = getCPUTemp(cpuTempPath);
      DPRINT("ATMP: sending temp - %d\n", temp);
      write(device, &temp, 4);
      break;
    }
    }
  }
}
int checkAliveStatus(char *devicePath) {
  FILE *device = fopen(devicePath, "r");
  if (device != NULL) {
    fclose(device);
    return 1;
  } else
    return 0;
}
char *cpuTempPath;
int main(int argc, char **argv) {
  if (argc < 2) {
    perror("need device argument");
    return 1;
  }
  // temp path init
  DPRINT("init cpu temperature path\n");
  while (cpuTempPath == NULL) {
    cpuTempPath = findCPUTempFile();
    usleep(100000);
  }
  DPRINT("got cpu temp path: %s\n", cpuTempPath);
  // find device
  int device = waitDevice(argv[1]);
  DPRINT("got the device\n");
  // waiting for arduino setup
  sleep(4);

  DPRINT("try to init a connection\n");
  int connectionTries = 0;
  while (1) {
    int connectionStatus = makeConnection(device);
    if (!connectionStatus && connectionTries < 100) {
      connectionTries++;
      usleep(10000);
    } else if (connectionStatus) {
      break;
    } else {
      perror("connection init failed");
      return 2;
    }
  }
  DPRINT("connection accepted\n");
  while (1) {
    if (!checkAliveStatus(argv[1])) {
      DPRINT("device has removed, closing the process\n");
      return 0;
    }
    listenDeviceCommand(device);
    usleep(100000);
  }

  DPRINT("closing the device...\n");
  close(device);
}
