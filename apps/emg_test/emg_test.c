#include <stdio.h>
#include <string.h>
#include <time.h>
#include "emg_driver.h"
#include "dsp.h"

int main(int argc, char* argv[])
{
    struct emg_driver* emg_config = emg_driver_init("/dev/rfcomm0");
    if (!emg_config)
    {
        return -1;
    }
    struct emg_data data;
    double data_array[4][10000];
    long timestamps_s[10000];
    long timestamps_ns[10000];
    for (int i = 0; i < 10000; i++)
    {
        emg_driver_get_samples(emg_config, &data);
        timestamps_s[i] = data.timestamp_s;
        timestamps_ns[i] = data.timestamp_ns;
        for (int j = 0; j < 4; j++)
        {
            data_array[j][i] = data.channels[j];
        }
        /*printf("%ld,%ld,%f,%f,%f,%f\n",
               data.timestamp_s,
               data.timestamp_ns,
               data.channels[0],
               data.channels[1],
               data.channels[2],
               data.channels[3]);*/
    }
    emg_driver_deinit(emg_config);

    double filtered_data[4][10000];
    for (int i = 0; i < 4; i++)
    {
        iir_filter(data_array[i], filtered_data[i], 10000);
    }

    for (int i = 0; i < 10000; i++)
    {
        printf("%ld,%ld,%f,%f,%f,%f\n",
               timestamps_s[i],
               timestamps_ns[i],
               filtered_data[0][i],
               filtered_data[1][i],
               filtered_data[2][i],
               filtered_data[3][i]);
    }
    
    return 0;
}
