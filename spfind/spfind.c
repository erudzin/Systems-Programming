/*******************************************************************************
 * Name        : spfind.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : March 30, 2021
 * Description : Sorted Permission Finder Implementation
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int pfind_to_sort[2], sort_to_parent[2];
    if(pipe(pfind_to_sort) < 0){
    	fprintf(stderr, "Error: Failed to create pipe pfind_to_sort. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    if(pipe(sort_to_parent) < 0){
    	fprintf(stderr, "Error: Failed to create pipe sort_to_parent. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

    pid_t pid[2];
    if ((pid[0] = fork()) < 0) {
    	fprintf(stderr, "Error: failed to fork. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    else if (pid[0] == 0) {
        // pfind
        close(pfind_to_sort[0]);
        
        if (dup2(pfind_to_sort[1], STDOUT_FILENO) < 0){
        	fprintf(stderr, "Error: failed to dup2 1.\n");
        	close(pfind_to_sort[1]);
        	close(sort_to_parent[0]);
        	close(sort_to_parent[1]);
        	return EXIT_FAILURE;
	}
        // Close all unrelated file descriptors.
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);

	// change the exec to run pfind with the list of arguments
        if (execvp("./pfind", argv) < 0){
            fprintf(stderr, "Error: pfind failed.\n");
            close(pfind_to_sort[1]);
            exit(EXIT_FAILURE);
        }
    }
    
    if ((pid[1] = fork()) < 0) {
    	fprintf(stderr, "Error: failed to fork. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    if (pid[1] == 0) {
        // sort
        close(pfind_to_sort[1]);
        if (dup2(pfind_to_sort[0], STDIN_FILENO) < 0){
       	 fprintf(stderr, "Error: failed to dup2 2.\n");
        	close(pfind_to_sort[0]);
        	close(sort_to_parent[0]);
        	close(sort_to_parent[1]);
        	return EXIT_FAILURE;
        }
        close(sort_to_parent[0]);
        if (dup2(sort_to_parent[1], STDOUT_FILENO) < 0){
        	fprintf(stderr, "Error: failed to dup2 3.\n");
        	close(sort_to_parent[1]);
        	close(pfind_to_sort[0]);
        	return EXIT_FAILURE;
        }

        // Close all unrelated file descriptors.

	//change the exec to run sort on the inputs from pfind
       if (execlp("sort", "sort", NULL) <0){
            fprintf(stderr, "Error: pfind failed.\n");
            exit(EXIT_FAILURE);
       }
    }


    //parent
    //do some more work in here
    close(sort_to_parent[1]);
    if (dup2(sort_to_parent[0], STDIN_FILENO) < 0){
    	fprintf(stderr, "Error: failed to dup2 4.\n");
    	close(sort_to_parent[0]);
    	close(pfind_to_sort[0]);
   	close(pfind_to_sort[1]);
    }
    // Close all unrelated file descriptors.
    close(pfind_to_sort[0]);
    close(pfind_to_sort[1]);
	
    ssize_t count;
    int counter = 0;
    char buffer[4096];
    while (1) {
    	if ((count = read(STDIN_FILENO, buffer, sizeof(buffer))) < 0){
    		fprintf(stderr, "Error: Failed to read from the pipe. %s\n", strerror(errno));
    		close(sort_to_parent[0]);
    		return EXIT_FAILURE;
    	}
        if (count == -1) {
            perror("read()");
            exit(EXIT_FAILURE);
        } else if(count == 0){
            break;
        } else {
            write(STDOUT_FILENO, buffer, count);
            for(int i=0; i<count; i++){
		if(buffer[i] == '\n'){
		    counter++;
		}
            }
     
        }
    }
    printf("Total Matches: %d\n",
            counter);

    close(sort_to_parent[0]);
    int status;
    if (waitpid(pid[0], &status, 0) < 0) {
    	fprintf(stderr, "Error: Wait failed for pfind. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    if (waitpid(pid[1], &status, 0) < 0) {
    	fprintf(stderr, "Error: Wait failed for sort. %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    
    return WEXITSTATUS(status);
}
