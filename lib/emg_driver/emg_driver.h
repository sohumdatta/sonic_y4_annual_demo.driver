#pragma once

struct emg_driver
{
    int fd;
};

struct emg_data
{
    time_t timestamp_s;
    long timestamp_ns;
    double channels[4];
};

struct emg_driver* emg_driver_init(char* dev_path);
void emg_driver_deinit(struct emg_driver* config);
void emg_driver_get_samples(struct emg_driver* config, struct emg_data *data);
