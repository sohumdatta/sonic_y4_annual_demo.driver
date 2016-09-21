#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
/* signal handling */
#include <signal.h>
#include <unistd.h>
/* Shared memory implementation */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
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
    struct shared* data_ptr = (struct shared*) NULL;
	
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

    int shmid;
    size_t filteredData_size = sizeof(filteredData);    
    
    /* create a shared memory (MUST CREATE)*/
    if((shmid = shm_open(SHARED_RESOURCE, O_CREAT | O_TRUNC | O_RDWR , 0600)) == -1)
    {perror("shm_open"); return;}
    ftruncate(shmid, sizeof(struct shared));

    /* attach to the shared memory (CAN READ OR WRITE)*/
    data_ptr = (struct shared *) mmap(0, sizeof(struct shared),
                PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if(data_ptr == (struct shared*) (-1)) {perror("mmap"); return;}

    /**** initialized shared resource ****/
    /* initialize structure values */
    data_ptr->filteredData.sec_elapsed = 0; 
    data_ptr->filteredData.ms_elapsed = 0; 
    data_ptr->filteredData.us_elapsed = 0; 
    for(j=0;j<4;j++) data_ptr->filteredData.channels[j] = 0.0; 


    /* make sure the mutex can be shared across processes */
    pthread_mutexattr_t shared_mutex_attr;
    pthread_mutexattr_init(&shared_mutex_attr);
    pthread_mutexattr_setpshared(&shared_mutex_attr, PTHREAD_PROCESS_SHARED);

    /* setup threads */
    pthread_mutex_init(&(data_ptr->mutex), &shared_mutex_attr);


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


    printf("Collecting and processing EMG data, press Ctrl-C to stop.\n");
    signal(SIGINT, sighandler);
    
    while(collect)
    {
        emg_driver_get_samples(emg_config, &data);

        sec_elapsed = data.sec_elapsed;
        ms_elapsed = data.ms_elapsed;
        us_elapsed = data.us_elapsed;


        /*******************************************
        *  CRITICAL SECTION BEGINS
        ********************************************/
        /* acquire MUTEX for critical section */
        pthread_mutex_lock(&(data_ptr->mutex));

        /* assign timing info to shared resource */
        data_ptr->filteredData.sec_elapsed = sec_elapsed;
        data_ptr->filteredData.ms_elapsed = ms_elapsed;
        data_ptr->filteredData.us_elapsed = us_elapsed;

        for (j = 0; j < 4; j++)
        {
            data_array[j] = data.channels[j];
            filtered_data_array[j] = iir_filter(data_array[j], &iir_state[j]);
            
            /* assign filtered channel values to shared resource */
            data_ptr->filteredData.channels[j] = filtered_data_array[j]; 

            /* assign raw channel values to shared resource */
            data_ptr->filteredData.raw_channels[j] = data_array[j]; 
        }
        /* release MUTEX on exiting critical section */
        pthread_mutex_unlock(&(data_ptr->mutex));
        /*******************************************
        *  CRITICAL SECTION ENDS
        ********************************************/


        /* copy to shared memory so that another process can read it */
        /* memcpy(data_ptr, &filteredData, filteredData_size); */

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
    
        /* this is required for safe file flushing*/
        fflush(fp);
    } /* while(1) */
    printf("\n\nInterrupt signal caught, closing file and bluetooth controller.\n");

    if(fclose(fp) == -1) {perror("Error closing output file"); return;}

    /* detach the shared memory */
    if(munmap(data_ptr, sizeof(struct shared*)) == -1) {perror("munmap"); return;} 

    /* destroy shmid for shared memory */
    if(shm_unlink(SHARED_RESOURCE) == -1) {perror("shm_unlink"); return;} 
    
    /* de-initialize bluetooth driver */
    emg_driver_deinit(emg_config); 
    printf("Data in '%s'\nPlease save it as it will be overwritten before re-run.\n", OUTPUT_FILE);
}
