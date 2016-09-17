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


/* Filter constants */
static const double default_b[7] = {0.941111105720312, -5.64666663432187, 14.1166665858047, -18.8222221144062, 14.1166665858047, -5.64666663432187, 0.941111105720312};
static const int default_b_length = 7;
static const double default_a[6] = {-5.87861915966800, 14.4004405303014, -18.8152897328414, 13.8294247398911, -5.42164649008798, 0.885690113310109};
static const int default_a_length = 6;

static double x_values[7];
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


void iir_filter(double input[], double output[], int length)
{
    int input_length, i;
    double bo = 0.0;
    double ao = 0.0;

    for (i = 0; i < length; i++)
    {
        ao = fir_filter(input, i, default_a, default_a_length);
        bo = fir_filter(input, i, default_b, default_b_length);
        output[i] = bo - ao;
    }
}
