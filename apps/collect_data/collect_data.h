/* collection of setup files used by the collection C routine */

/* file including path */
#define OUTPUT_FILE "~/sonic_y4_annual_demo/bluetooth/run/latest_emg_data.txt"

/* structure to save the filtered data with the time stamps */
struct filtered_data{
    long sec_elapsed;
    int ms_elapsed;
    int us_elapsed;
    double channels[4];
};
