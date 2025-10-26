#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUDRATE B9600

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;

    char *mode = argv[1];
    char *param = argv[2];

    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);

    char msg[128] = {0};
    strcpy(msg, mode);
    strcat(msg, " ");
    strcat(msg, param);
    for (int i = 3; i < argc; i++) {
        strcat(msg, " ");
        strcat(msg, argv[i]);
    }
    strcat(msg, "\r\n");  // CR+LF로 수정

    if (strcmp(mode, "camera") == 0 && strcmp(param, "True") == 0) {
        system("sudo v4l2-ctl --device=/dev/video0 "
               "--set-fmt-video=width=1920,height=1080,pixelformat=MJPG "
               "--stream-mmap --stream-count=1 "
               "--stream-to=/var/www/html/capture.jpg");
    }

    write(fd, msg, strlen(msg));
    close(fd);
    printf("Sent to Arduino: %s\n", param);
    return 0;
}
