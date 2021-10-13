/*******************************************************************************
 * Name        : sort.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : March 1, 2021
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN 64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum
{
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Reads data from filename into an already allocated 2D array of chars.
 * Exits the entire program if the file cannot be opened.
 */
size_t read_data(char *filename, char **data)
{
    // Open the file.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", filename,
                strerror(errno));
        free(data);
        exit(EXIT_FAILURE);
    }

    // Read in the data.
    size_t index = 0;
    char str[MAX_STRLEN + 2];
    char *eoln;
    while (fgets(str, MAX_STRLEN + 2, fp) != NULL)
    {
        eoln = strchr(str, '\n');
        if (eoln == NULL)
        {
            str[MAX_STRLEN] = '\0';
        }
        else
        {
            *eoln = '\0';
        }
        // Ignore blank lines.
        if (strlen(str) != 0)
        {
            data[index] = (char *)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(data[index++], str);
        }
    }

    // Close the file before returning from the function.
    fclose(fp);

    return index;
}


/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv)
{
    char opt;
    int flag_i = 0;
    int flag_d = 0;
    char usage[] = "Usage: ./sort [-i|-d] filename\n   -i: Specifies the file contains ints.\n   -d: Specifies the file contains doubles.\n   filename: The file to sort.\n   No flags defaults to sorting strings.";
    if(argc <= 1){
        fprintf(stderr, "%s\n", usage);
        return EXIT_FAILURE;
    }
    
    while ((opt = getopt(argc, argv, ":id")) != -1)
    {
        switch (opt)
        {
        case 'i':
            flag_i = 1;
            break;
        case 'd':
            flag_d = 1;
            break;
        case '?':
            fprintf(stderr, "Error: Unknown option '-%c' received.\n%s\n", optopt, usage);
            return EXIT_FAILURE;
        case ':':
            fprintf(stderr, "Error: No input file specified.\n");
            return EXIT_FAILURE;
        }
    }
    
    if(flag_i == 1 && flag_d == 1){
    	fprintf(stderr, "Error: Too many flags specified.\n");
    	return EXIT_FAILURE;
    }
    if(flag_i == 1 || flag_d == 1){
    	if(argc > 3){
    		fprintf(stderr, "Error: Too many files specified.\n");
    		return EXIT_FAILURE;
    	}
        if(argc < 3){
            fprintf(stderr, "Error: No input file specified.\n");
            return EXIT_FAILURE;
        }
    }
    else {
    	if(argc > 2){
    		fprintf(stderr, "Error: Too many files specified.\n");
    		return EXIT_FAILURE;
    	}
    }

    char **my_data = (char **)malloc(sizeof(char *) * MAX_ELEMENTS);

    int size_of_array = read_data(argv[argc-1], my_data);
    
    if(flag_i == 1){
        int *int_list = (int *)malloc(sizeof(int) * size_of_array);
        for(int i = 0; i < size_of_array; i++){
            int_list[i] = atoi(my_data[i]);
        }
        quicksort(int_list, size_of_array, sizeof(int), int_cmp);
        for(int i = 0; i < size_of_array; i++){
            printf("%d\n", int_list[i]);
        }
        free(int_list);
    }

    else if(flag_d == 1){
        double *double_list = (double *)malloc(sizeof(double) * size_of_array);
        for(int i = 0; i < size_of_array; i++){
            double_list[i] = atof(my_data[i]);
        }
        quicksort(double_list, size_of_array, sizeof(double), dbl_cmp);
        for(int i = 0; i < size_of_array; i++){
            printf("%f\n", double_list[i]);
        }
        free(double_list);
    }
    
    else{
        char **string_list = (char **)malloc(sizeof(char*) * size_of_array);
        for(int i = 0; i < size_of_array; i++){
            string_list[i] = my_data[i];
        }
        quicksort(string_list, size_of_array, sizeof(char*), str_cmp);
        for(int i = 0; i < size_of_array; i++){
            printf("%s\n", string_list[i]);
        }
        free(string_list);
    }
    for(int i = 0; i < size_of_array; i++){
        free(my_data[i]);
    }
    free(my_data);

    return EXIT_SUCCESS;
}

