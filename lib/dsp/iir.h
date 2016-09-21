#pragma once

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
 *
 * NOTE: The application must allocate pointers to x_values and y_values, 
 * and setup x_len and y_len accordingly
 *
 */

struct iir_state_t{
	double *  x_values;
	double *  y_values;
	int x_len;
	int y_len;
};

double iir_filter(double input, struct iir_state_t* iir_state);
/* To get the latest value of output y, just search for y[0] */

/* This isn't the best way to pass lengths of arrays, but for this limited application macros will do */
#define X_LEN 7
#define Y_LEN 6
