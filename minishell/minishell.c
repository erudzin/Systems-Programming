/*******************************************************************************
 * Name        : minishell.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : April 12, 2021
 * Description : Minishell Implementation
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <stdbool.h>
#include <signal.h>
#include <wait.h>

#define BUFSIZE 4096
#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

volatile sig_atomic_t interrupted = false;

void catch_signal(int sig)
{
	interrupted = sig;
	write(STDOUT_FILENO, "\n", 1);
	interrupted = false;
//	goto CLEANUP;
}

void prompt(char *cwd)
{
	printf("[%s%s%s]$ ", BRIGHTBLUE, cwd, DEFAULT);
}

void change_dir(char *path)
{
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);

	// if changing directory to home
	if (!(strcmp(path, "~")))
	{
		if (chdir(pw->pw_dir) != 0)
		{
			fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", pw->pw_dir, strerror(errno));
		}
	}
	else if (strncmp(path, "~/", 2) == 0)
	{
		char my_dir[PATH_MAX];
		sprintf(my_dir, "%s%s", pw->pw_dir, path + 1);
		if (chdir(my_dir) != 0)
		{
			fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", my_dir, strerror(errno));
		}
	}
	else
	{
		// change directory to anything else
		if (chdir(path) != 0)
		{
			fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
		}
	}
}

int main()
{

	char cwd[PATH_MAX];
	char buffer[BUFSIZE];
	int read_chars;



	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = catch_signal;
	//figure out what this should be instead of SA_RESTART

	if (sigaction(SIGINT, &action, NULL) == -1)
	{
		fprintf(stderr, "Error: Cannot register signal handler. %s.\n",
				strerror(errno));
		return EXIT_FAILURE;
	}
	//LOOP:
	while (true)
	{
		if (!interrupted)
		{	
			buffer[0] = '\0';
			//bool more_than_one_word = false;
			// get current directory
			if (getcwd(cwd, sizeof(cwd)) == NULL)
			{
				fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
			}
			// print current dirrectory waiting for command
			prompt(cwd);
			fflush(stdout);
			// read command into buffer and update how many chars read
			if ((read_chars = read(STDIN_FILENO, buffer, BUFSIZE)) < 0)
			{
				if (errno != EINTR)
				{
					fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
					return EXIT_FAILURE;
				}
			}
			buffer[read_chars - 1] = '\0';

			//printf("%s", buffer);
			//if (read_chars != 0){

			//}
			//break the read command into individual words in an array

			//remember to free these
			//remember to free these
			char *input_array[2048];
			char *token = strtok(buffer, " ");
			int num_words = 0;
			while (token != NULL)
			{
				input_array[num_words] = strdup(token);
				num_words++;
				token = strtok(NULL, " ");
			}
			input_array[num_words] = NULL;
			if (num_words == 0)
			{
				continue;
			}

			//check if comand is exit
			if (!strcmp(input_array[0], "exit"))
			{
				for (int i = 0; i < num_words; i++)
				{
					free(input_array[i]);
				}

				return EXIT_SUCCESS;
			}
			// check if comand is cd
			else if (!strcmp(input_array[0], "cd"))
			{
				if (num_words > 2)
				{
					printf("Error: Too many arguments to cd. \n");
					continue;
				}
				else
				{
					if (num_words == 1)
					{
						change_dir("~");
					}
					else
					{
						change_dir(input_array[1]);
					}
				}

				for (int i = 0; i < num_words; i++)
				{
					free(input_array[i]);
				}
				continue;
			}
			//other commands
			pid_t pid;
			if ((pid = fork()) < 0)
			{
				fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}
			if (pid > 0)
			{ // In parent process
				int status;
				if (waitpid(pid, &status, WUNTRACED | WCONTINUED) < 0)
				{

					if (errno != EINTR)
					{
						fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
						return EXIT_FAILURE;
					}
				}
			}
			else if (pid == 0)
			{ // In child process
				if (execvp(input_array[0], input_array) == -1)
				{
					fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
					return EXIT_FAILURE;
				}
				for (int i = 0; i < num_words; i++)
			{
				free(input_array[i]);
			}
			}

			for (int i = 0; i < num_words; i++)
			{
				free(input_array[i]);
			}
		}
		else
		{
			interrupted = false;
		}
	}

	//		printf("%s",buffer);
	//CLEANUP:
	//for (int i = 0; i < num_words; i++){
	//	free(input_array[i]);
	//}
	//goto LOOP;
}

