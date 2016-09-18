#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "emg_driver.h"
#include "dsp.h"
#include "collect_data.h"


void main(int argc, char* argv[])
{
    
    FILE * fp = (FILE*) NULL;  /* file pointer to the output file */
    struct emg_driver* emg_config = (struct emg_driver*) NULL;  /* emg driver config */
    int i, j;
    

    errno = 0;
    fp = fopen(OUTPUT_FILE, "w+");
    if(fp == NULL){
        perror("Error opening file:");
        return;
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


    printf("Printing raw and processed values to file '%s', press Ctrl-C to stop.\n", OUTPUT_FILE);

    while(1)
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
}
