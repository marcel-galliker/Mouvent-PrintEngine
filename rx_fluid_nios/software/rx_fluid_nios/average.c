/***************************************************************************//**
 \file average.c

 Calculate average from a small array of integers
 
 \author 	Cyril Andreatta
 
 \copyright 2017 by radex AG, Switzerland. All rights reserved.
*******************************************************************************/

#include "average.h"

#ifdef AVERAGE
/** 
 * \brief Selection sort algorithm
 *
 * \param array     Pointer to array to sort
 * \param len       Length of array
 **/
void selection_sort(INT32 *array, int len)
{
    int x = 0;
    int y = 0;
    int min = 0;
    int temp = 0;
    
	for(x = 0; x < len; x++) {
		min = x;

		for(y = x; y < len; y++) {
			if(array[min] > array[y]) {
				min = y;
			}
		}
		// swap
		temp = array[x];
		array[x] = array[min];
		array[min] = temp;
	}
}

/** 
 * \brief Calculate average from an array while ignoring smallest/largest values
 *
 * \param buffer    Pointer to array (does not have to be sorted to begin with)
 * \param len       Length of array
 * \param cut       Number of array items to cut from beginning and end
 *
 * \return calculated average value
 **/
INT32 average (INT32 *buffer, unsigned int len, unsigned int cut)
{
    int i = 0;
	INT64 sum = 0;
	
	if (cut >= len / 2)
		cut = 0;
    else
        selection_sort(buffer, len);
	
	for (i = cut; i < (len - cut); i++)
		sum += buffer[i];

	sum /= (len - (cut << 1));
	
	return (INT32)sum;
}
#endif
