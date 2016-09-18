#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "emg_driver.h"
#include "dsp.h"
#include "collect_data.h"


/* signal handler, closes file written to and deinitializes emg driver */
/*void sighandler(int signum){
    printf("Caught an interrupt %d, closing write file and deinitializing Bluetooth.", signum);
    emg_driver_deinit(emg_config);
    fclose(fp); 
}
*/
void main(int argc, char* argv[])
{
    FILE * fp = (FILE*) NULL;  /* file pointer to the output file */
    struct emg_driver* emg_config = (struct emg_driver*) NULL;  /* emg driver config */
    
    int i, j;
    fp = fopen("dump", "w");
    while(fp == NULL){
         printf("Opening file %s failed retrying\n", OUTPUT_FILE);
        fp = fopen("dump", "w");
    }
    emg_config = emg_driver_init("/dev/rfcomm0");

    struct emg_data data;
    struct filtered_data filteredData;

    double data_array[4];
    double filtered_data_array[4];

    long sec_elapsed;
    int ms_elapsed;
    int us_elapsed;

    if (!emg_config)
    {
        printf("Error: emg driver not configured, exit!\n");
        return;
    }

    /* signal(SIGINT, sighandler); */
    while(1)
    {
        emg_driver_get_samples(emg_config, &data);

/*        timestamps_s[i] = data.timestamp_s;
        timestamps_ns[i] = data.timestamp_ns;
*/
        sec_elapsed = data.sec_elapsed;
        ms_elapsed = data.ms_elapsed;
        us_elapsed = data.us_elapsed;


        for (j = 0; j < 4; j++)
        {
            data_array[j] = data.channels[j];
            filtered_data_array[j] = iir_filter(data_array[j]);
/*            filteredData.channel[j] = iir_filter(data_array[j][i]); */
        }
        /*printf("%ld,%ld,%f,%f,%f,%f\n",
               data.timestamp_s,
               data.timestamp_ns,
               data.channels[0],
               data.channels[1],
               data.channels[2],
               data.channels[3]);*/


           fprintf(fp, "%ld,%d,%d,\t%f,%f,%f,%f,\t%f,%f,%f,%f\n",
               sec_elapsed,
               ms_elapsed,
               us_elapsed,
               data_array[0],
               data_array[1],
               data_array[2],
               data_array[3],
               filtered_data_array[0],
               filtered_data_array[1],
               filtered_data_array[2],
               filtered_data_array[3]);
        printf("reaches here\n"); /**** PROBE ****/
    } /* while(1) */
}
