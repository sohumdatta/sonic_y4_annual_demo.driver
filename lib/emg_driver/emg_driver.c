#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "emg_driver.h"

/* Useful constants */
static const uint8_t DLE = 0x10;
static const uint8_t SOH = 0x01;
static const uint8_t EOT = 0x04;
static const speed_t BAUDRATE = B460800;

/* Helper functions */

static char read_byte(int fd)
{
    char data = 0;
    while (!read(fd, &data, 1));
    return data;
}

static double parse_data(char packet2, char packet3, char packet4)
{
    long num;
    double temp_voltage;

    num = packet2 * 65536L + packet3 * 256L + packet4;
    
    if (num < 8388607)
    {
        temp_voltage = (num * 3.3 * 1000) / 8388607.0;
    }
    else
    {
        num = (16777215 - num) + 1;
        temp_voltage = num;
        temp_voltage = (num * 3.3 * 1000) / 8388607.0;
        
        temp_voltage *= -1;
    }
    
    return temp_voltage;
}

/* Library functions */

struct emg_driver* emg_driver_init(char* dev_path)
{
    struct emg_driver* config = malloc(sizeof(struct emg_driver));
    config->fd = open(dev_path, O_RDWR | O_NOCTTY | O_NDELAY); /* Open in non-blocking mode */
    if (config->fd == -1)
    {
        free(config);
        printf("Error: could not open %s for serial communication.\n", dev_path);
        return NULL;
    }

    struct termios options;
    tcgetattr(config->fd, &options);
    options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; /* 8N1 */
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(config->fd, TCIFLUSH);
    tcsetattr(config->fd, TCSANOW, &options);

    /* Initialize EMG sensor */
    unsigned char tx_buffer = 'A';
    int count = write(config->fd, &tx_buffer, 1);
    if (count < 0)
    {
        printf("Serial TX error\n");
        free(config);
        return NULL;
    }

    return config;
}

void emg_driver_deinit(struct emg_driver* config)
{
    close(config->fd);
    free(config);
}

void emg_driver_get_samples(struct emg_driver* config, struct emg_data *data)
{
    /*tcflush(config->fd, TCIFLUSH); /* Flush */

    char c_data = 0;
    char p_data = 0;
    char packet[256];
    memset(packet, 0, 256);
    int data_count = 0;
    while (((p_data != DLE) || (c_data != SOH)))
    {
        p_data = c_data;
        c_data = read_byte(config->fd);
        if ((p_data == DLE) && (c_data == DLE))
        {
            p_data = c_data;
            c_data = read_byte(config->fd);
        }
    }
    while (((p_data != DLE) || (c_data != EOT)))
    {
        p_data = c_data;
        c_data = read_byte(config->fd);

        if (c_data != DLE)
        {
            packet[(data_count++) % 256] = c_data;
        }
        else
        {
            p_data = c_data;
            c_data = read_byte(config->fd);
            if (c_data == DLE)
            {
                packet[(data_count++) % 256] = c_data;
            }
        }
    }

    if (data_count == 12) /* 12 bytes of data per packet */
    {
        struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        data->timestamp_s = spec.tv_sec;
        data->timestamp_ns = spec.tv_nsec;
        for (int i = 0; i < 4; i++)
        {
            data->channels[i] = parse_data(packet[3 * i],
                                           packet[3 * i + 1],
                                           packet[3 * i + 2]);
        }
   }
}
