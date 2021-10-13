/*******************************************************************************
 * Name        : quicksort.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : March 1, 2021
 * Description : Quicksort implementation.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "quicksort.h"

/* Static (private to this file) function prototypes. */
static void swap(void *a, void *b, size_t size);
static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*));
static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*));

/**
 * Compares two integers passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to int pointers.
 * Returns:
 * -- 0 if the integers are equal
 * -- a positive number if the first integer is greater
 * -- a negative if the second integer is greater
 */
int int_cmp(const void *a, const void *b) {
	return *(const int *)a - *(const int *)b;
}

/**
 * Compares two doubles passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to double pointers.
 * Returns:
 * -- 0 if the doubles are equal
 * -- 1 if the first double is greater
 * -- -1 if the second double is greater
 */
int dbl_cmp(const void *a, const void *b) {
//	return *(const double *)a - *(const double *)b;
  double *a_dbl = (double *)a;
  double *b_dbl = (double *)b;
  if ((*a_dbl - *b_dbl) == 0) {
    return 0;
  }
  else if (*a_dbl > *b_dbl) {
    return 1;
  }
  else {
    return -1;
  }
}

/**
 * Compares two char arrays passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to char* pointers (i.e. char**).
 * Returns the result of calling strcmp on them.
 */
int str_cmp(const void *a, const void *b) { 
	return strcmp(*(const char **)a, *(const char **)b);
}

/**
 * Swaps the values in two pointers.
 *
 * Casts the void pointers to character types and works with them as char
 * pointers for the remainder of the function.
 * Swaps one byte at a time, until all 'size' bytes have been swapped.
 * For example, if ints are passed in, size will be 4. Therefore, this function
 * swaps 4 bytes in a and b character pointers.
 */
static void swap(void *a, void *b, size_t size) {
    char * char_a = (char *)a;
    char * char_b = (char *)b;
    char temp;
    for(int i = 0; i < size; i++){
        temp = char_a[i];
        char_a[i] = char_b[i];
        char_b[i] = temp;
    }


}

/**
 * Partitions array around a pivot, utilizing the swap function.
 * Each time the function runs, the pivot is placed into the correct index of
 * the array in sorted order. All elements less than the pivot should
 * be to its left, and all elements greater than or equal to the pivot should be
 * to its right.
 * The function pointer is dereferenced when it is used.
 * Indexing into void *array does not work. All work must be performed with
 * pointer arithmetic.
 */
static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*)) {
	char *char_arr = array;
	void *p = char_arr + left*elem_sz;
	int l = left;
	for (size_t i = left+1; i<= right; i++){
		if(comp((char *) array + i*elem_sz, p) < 0){
			l++;
			swap((char *)array + l*elem_sz, (char *)array + i*elem_sz, elem_sz);
		}
	}
	swap((char *)array + l*elem_sz, (char *)array + left*elem_sz, elem_sz);
	return l;
}

/**
 * Sorts with lomuto partitioning, with recursive calls on each side of the
 * pivot.
 * This is the function that does the work, since it takes in both left and
 * right index values.
 */
static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*)) {
    if (left<right){
    	int p = lomuto(array, left, right, elem_sz, comp);
    	quicksort_helper(array, left, p-1, elem_sz, comp);
    	quicksort_helper(array, p+1, right, elem_sz, comp);
    }
}

/**
 * Quicksort function exposed to the user.
 * Calls quicksort_helper with left = 0 and right = len - 1.
 */
void quicksort(void *array, size_t len, size_t elem_sz,
               int (*comp) (const void*, const void*)) {
	quicksort_helper(array, 0, len-1, elem_sz, comp);
}
