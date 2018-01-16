#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>

int main()
{
    int fd;
    int ads_address = 0x48;
    uint8_t buf[10];
    int16_t val;

    // Open device on /dev/i2c-1 the default on Raspberry Pi B
    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        printf("Error: Couldn't open device! %d\n", fd);
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, ads_address) < 0)
    {
        printf("Error: Couldn't find device on address!\n");
        return 1;
    }

    // set config register and start conversion
    // AIN0 and GND, 4.096V, 128s/s
    buf[0] = 1; // config register is 1
    buf[1] = 0xc3;
    buf[2] = 0x85;
    if (write(fd, buf, 3) != 3)
    {
        perror("Write to register 1");
        exit(-1);
    }

    // wait for conversion complete
    do
    {
        if (read(fd, buf, 2) != 2)
        {
            perror("Read conversion");
            exit(-1);
        }
    } while (buf[0] & 0x80 == 0);

    // read conversion register
    buf[0] = 0; // Conversion register is 0
    if (write(fd, buf, 1) != 1)
    {
        perror("Write register select");
        exit(-1);
    }
    if (read(fd, buf, 2) != 2)
    {
        perror("Read conversion");
        exit(-1);
    }

    // convert output and display results
    val = (int16_t)buf[0]*256 + (uint16_t)buf[1];
    printf("Conversion %02x %02x %d %f\n",
            buf[0], buf[1], val, (float)val*4.096/32768.0);

    close(fd);

    return 0;
}
