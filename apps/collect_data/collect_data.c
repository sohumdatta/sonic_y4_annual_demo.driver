#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/* signal handling */
#include <signal.h>
#include <unistd.h>
/* Shared memory implementation */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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
    struct filtered_data* data_ptr = (struct filtered_data*) NULL;

    int shmid;
    size_t filteredData_size = sizeof(filteredData);    
    
    /* Generate key for sharing filteredData */
    errno = 0;
    key_t key = ftok(OUTPUT_FILE, 'R');
    if(key == -1) {perror("ftok"); return;}
    
    /* create a shared memory (MAY CREATE)*/
    if((shmid = shmget(key, filteredData_size, 0644 | IPC_CREAT | IPC_EXCL)) == -1)
    {perror("shmget"); return;}

    /* attach to the shared memory (CAN READ OR WRITE)*/
    data_ptr = (struct filtered_data*) shmat(shmid, (void*) 0, 0);
    if(data_ptr == (struct filtered_data*) (-1)) {perror("shmat"); return;}


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

        filteredData.sec_elapsed = sec_elapsed;
        filteredData.ms_elapsed = ms_elapsed;
        filteredData.us_elapsed = us_elapsed;

        for (j = 0; j < 4; j++)
        {
            data_array[j] = data.channels[j];
            filtered_data_array[j] = iir_filter(data_array[j]);
            filteredData.channels[j] = iir_filter(data_array[j]); 
        }

        /* copy to shared memory so that another process can read it */
        memcpy(data_ptr, &filteredData, filteredData_size);

        /* print raw values collected to the file */
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

    if(fclose(fp) == -1) {perror("Error closing output file"); return;}

    /* detach the shred memory */
    if(shmdt(data_ptr) == -1) {perror("shmdt"); return;} 

    /* destroy shmid for shared memory */
    if(shmctl(shmid, IPC_RMID, (void*) NULL) == -1) {perror("shmctl"); return;} 
    
    /* de-initialize bluetooth driver */
    emg_driver_deinit(emg_config); 
    printf("Data in '%s'\nPlease save it as it will be overwritten before re-run.\n", OUTPUT_FILE);
}
