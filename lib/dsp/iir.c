#include "iir.h"

/* Useful constants */

static const double default_b[7] = {0.941111105720312, -5.64666663432187, 14.1166665858047, -18.8222221144062, 14.1166665858047, -5.64666663432187, 0.941111105720312};
static const int default_b_length = 7;
static const double default_a[6] = {-5.87861915966800, 14.4004405303014, -18.8152897328414, 13.8294247398911, -5.42164649008798, 0.885690113310109};
static const int default_a_length = 6;


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
