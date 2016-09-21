#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "emg_driver.h"
#include "dsp.h"

int main(int argc, char* argv[])
{
    int i, j;
    struct emg_driver* emg_config = emg_driver_init("/dev/rfcomm0");
    if (!emg_config)
    {
        return -1;
    }
    struct emg_data data;
	struct iir_state_t iir_state[4];	/* state values stored for the 4 channels */
	/* initialize the stored initial channel values to zeros */
	for(i=0; i < 4; i++){
		iir_state[i].x_values = (double *) malloc(X_LEN * sizeof(double));
		iir_state[i].x_len = X_LEN;

		iir_state[i].y_values = (double *) malloc(Y_LEN * sizeof(double));
		iir_state[i].y_len = Y_LEN;

		for(j=0; j < iir_state[i].x_len; j++) iir_state[i].x_values[j] = 0.0;
		for(j=0; j < iir_state[i].y_len; j++) iir_state[i].y_values[j] = 0.0;
	}


    double data_array[4][10000];
    double filtered_data[4][10000];

    long timestamps_s[10000];
    long timestamps_ns[10000];

    long sec_elapsed[10000];
    int ms_elapsed[10000];
    int us_elapsed[10000];

    for (i = 0; i < 10000; i++)
    {
        emg_driver_get_samples(emg_config, &data);

        timestamps_s[i] = data.timestamp_s;
        timestamps_ns[i] = data.timestamp_ns;

        sec_elapsed[i] = data.sec_elapsed;
        ms_elapsed[i] = data.ms_elapsed;
        us_elapsed[i] = data.us_elapsed;

        for (j = 0; j < 4; j++)
        {
            data_array[j][i] = data.channels[j];
            filtered_data[j][i] = iir_filter(data_array[j][i], &iir_state[j]);
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

/*    for (i = 0; i < 4; i++)
    {
        iir_filter(data_array[i], filtered_data[i], 10000);
    }
*/
    for (i = 0; i < 10000; i++)
    {
/*
           printf("%ld,%ld,%f,%f,%f,%f\n",
               timestamps_s[i],
               timestamps_ns[i],
               filtered_data[0][i],
               filtered_data[1][i],
               filtered_data[2][i],
               filtered_data[3][i]);
*/
           printf("%ld,%d,%d,%f,%f,%f,%f\n",
               sec_elapsed[i],
               ms_elapsed[i],
               us_elapsed[i],
               filtered_data[0][i],
               filtered_data[1][i],
               filtered_data[2][i],
               filtered_data[3][i]);
    }
    
    return 0;
}
