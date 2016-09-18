#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "emg_driver.h"
#include "dsp.h"
#include "collect_data.h"

int collect = 1;

void sighandler(int signum){ collect = 0;}

void main(int argc, char* argv[])
{
    
    FILE * fp = (FILE*) NULL;  /* file pointer to the output file */
    struct emg_driver* emg_config = (struct emg_driver*) NULL;  /* emg driver config */
    int i, j;
    
    struct emg_data data;
    struct filtered_data filteredData;

    double data_array[4];
    double filtered_data_array[4];

    long sec_elapsed;
    int ms_elapsed;
    int us_elapsed;

    errno = 0;
    fp = fopen(OUTPUT_FILE, "w+");
    if(fp == NULL){
        perror("Error opening output file");
        return;
    }

    emg_config = emg_driver_init("/dev/rfcomm0");

    if (!emg_config)
    {
        printf("Error: emg driver not configured, exit!\n");
        return;
    }


    printf("Collecting and processing EMG data, press Ctrl-C to stop.\n", OUTPUT_FILE);
    signal(SIGINT, sighandler);
    
    while(collect)
    {
        emg_driver_get_samples(emg_config, &data);

        sec_elapsed = data.sec_elapsed;
        ms_elapsed = data.ms_elapsed;
        us_elapsed = data.us_elapsed;


        for (j = 0; j < 4; j++)
        {
            data_array[j] = data.channels[j];
            filtered_data_array[j] = iir_filter(data_array[j]);
/*            filteredData.channel[j] = iir_filter(data_array[j][i]); */
        }

           fprintf(fp, "%ld,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f\n",
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
    } /* while(1) */
    printf("\n\nInterrupt signal caught, closing file and bluetooth controller.\n");

    errno = 0;
    if(fclose(fp) == -1) {perror("Error closing output file"); return;}

    emg_driver_deinit(emg_config); 
    printf("Data in '%s'\nPlease save it as it will be overwritten before re-run.\n", OUTPUT_FILE);
}
