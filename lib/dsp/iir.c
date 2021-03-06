#include "iir.h"
/*
 * IIR Implementation:
 *
 * y[n] represents the output and x[n] the intput to the filter at instant n.
 * The output of the filter is 
 *
 * y[n] = (b[0] x[n] + b[1] x[n-1] + ... + b[6] x[n-6]) - (a[0] y[n-1] + a[1] y[n-2] + ... + a[5] y [n-6])
 *
 * note that bo below is the first term, and ao is the second term
 */

/*
 * x_values and y_values are arrays that store the latest values of x and y.
 * both are implemented as a queue, x_values[0] = x[n], x_values[1] = x[n-1]...
 * similarly, y_values[0] = y[n-1], y_values[1] = y[n-2] ...
 * 
 * then, 
 * bo = b[0] x_values[0] + b[1] x_values[1] + ...
 * ao = a[0] y_values[0] + a[1] y_values[1] + ...
 */

/* Filter constants */
static const double default_b[7] = {0.941111105720312, -5.64666663432187, 14.1166665858047, -18.8222221144062, 14.1166665858047, -5.64666663432187, 0.941111105720312};
static const int default_b_length = 7;
static double x_values[7];

static const double default_a[6] = {-5.87861915966800, 14.4004405303014, -18.8152897328414, 13.8294247398911, -5.42164649008798, 0.885690113310109};
static const int default_a_length = 6;
static double y_values[6];


/* Helper functions */

static int min(int a, int b)
{
    return (a < b) ? a : b;
}

static double fir_filter(double state[], const int state_length, const double coefficients[], const int coefficients_length)
{
    int i;
    double output = 0.0;
    
    for (i = 0; i < min(state_length, coefficients_length); i++)
    {
        output += state[i] * coefficients[i];
    }

    return output;
}


/*
 * The IIR function below take a structure that stores the most recent 7 x_values and 6 y_values (save one of this instant)
 * it also takes in the current input value, and returns the current output value.
 * Note that the structure contents are updated during the run.
 *
 * x_values and y_values are arrays that store the latest values of x and y.
 * both are implemented as a queue, x_values[0] = x[n], x_values[1] = x[n-1]...
 * similarly, y_values[0] = y[n-1], y_values[1] = y[n-2] ...
 * 
 * then, 
 * bo = b[0] x_values[0] + b[1] x_values[1] + ...
 * ao = a[0] y_values[0] + a[1] y_values[1] + ...
 */

double iir_filter(double input, struct iir_state_t * iir_state)
{
    
    int i;
    double output = 0.0;
    double bo = 0.0;
    double ao = 0.0;

    /* shift the inputs for input data */
    for (i = iir_state->x_len; i > 0; i--) iir_state->x_values[i] = iir_state->x_values[i-1];
    	iir_state->x_values[0] = input;

    bo = fir_filter(iir_state->x_values, iir_state->x_len, default_b, default_b_length);
    ao = fir_filter(iir_state->y_values, iir_state->y_len, default_a, default_a_length);
    output = bo - ao;

    /* shift the outputs for output data, for the next iteration */
    for (i = iir_state->y_len; i > 0; i--) iir_state->y_values[i] = iir_state->y_values[i-1];
    iir_state->y_values[0] = output;

    return output;
}
