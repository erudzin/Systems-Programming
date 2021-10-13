/*******************************************************************************
 * Name        : pfind.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : March 15, 2021
 * Description : Permission Finder Implementation
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>

int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

char *user_dir;
char *user_perm;
int user_perm_to_int;

bool verify_string(char *permissions)
{
    if (strlen(permissions) != 9)
    {
        return false;
    }
    for (int i = 0; i < 9; i += 3)
    {
        if (!(permissions[i] == 'r' || permissions[i] == '-'))
        {
            return false;
        }
        if (!(permissions[i + 1] == 'w' || permissions[i + 1] == '-'))
        {
            return false;
        }
        if (!(permissions[i + 2] == 'x' || permissions[i + 2] == '-'))
        {
            return false;
        }
    }
    return true;
}

char* permission_string(struct stat *statbuf) {
    int permission_valid;
    char *return_string = (char *)malloc(sizeof(char) * 11);
    return_string[0] = '-';
    for (int i = 0; i < 9; i += 3) {
        permission_valid = statbuf->st_mode & perms[i];
        if (permission_valid) {
            return_string[i+1] = 'r';
        } else {
            return_string[i+1] = '-';
        }
        permission_valid = statbuf->st_mode & perms[i+1];
        if (permission_valid) {
            return_string[i+2] = 'w';
        } else {
            return_string[i+2] = '-';
         }
        permission_valid = statbuf->st_mode & perms[i+2];
        if (permission_valid) {
            return_string[i+3] = 'x';
        } else {
            return_string[i+3] = '-';
         }
    }
    return_string[11] = '\n';
    
    return return_string;

}

int perm_int(char *perms)
{
    bool ten;
    int div = 1;
    int temp = 0;
    int result = 0;
    if (strlen(perms) == 10)
    {
        ten = true;
    }
    else
    {
        div = 2;
        ten = false;
    }

    for (int i = 0; i < 10; i = i + 3)
    {
        if (i == 0 && ten)
        {
            i++;
        }

        if (perms[i] == 'r')
        {
            temp = 1;
            for (int j = i + 1; j < 10; j++)
            {
                temp = temp * 2;
            }
            result += temp / div;
        }
        if (perms[i + 1] == 'w')
        {
            temp = 1;
            for (int j = i + 2; j < 10; j++)
            {
                temp = temp * 2;
            }
            result += temp / div;
        }
        if (perms[i + 2] == 'x')
        {
            temp = 1;
            for (int j = i + 3; j < 10; j++)
            {
                temp = temp * 2;
            }
            result += temp / div;
        }
    }
    return result;
}

int nav_tree(char *path)
{
    DIR *dir;
    if ((dir = opendir(path)) == NULL)
    {
        fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", path, strerror(errno));
        return EXIT_FAILURE;
    }
    struct dirent *entry;
    struct stat sb;
    char full_filename[PATH_MAX + 1];
    size_t pathlen = 0;
    // Set the initial character to the NULL byte.
    // If the path is root '/', you can now take the strlen of a properly
    // terminated empty string.
    full_filename[0] = '\0';
    if (strcmp(path, "/"))
    {
        // If path is not the root - '/', then ...
        // If there is no NULL byte among the first n bytes of path,
        // the full_filename will not be terminated. So, copy up to and
        // including PATH_MAX characters.
        strncpy(full_filename, path, PATH_MAX);
    }
    // Add + 1 for the trailing '/' that we're going to add.
    pathlen = strlen(full_filename) + 1;
    full_filename[pathlen - 1] = '/';
    full_filename[pathlen] = '\0';
    while ((entry = readdir(dir)) != NULL)
    {
    	if (!strcmp(entry->d_name, ".") ||
            !strcmp(entry->d_name, "..")) {
      		continue;
    	}
    	strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
    	if (lstat(full_filename, &sb) < 0) {
            fprintf(stderr, "Error: Cannot stat '%s'. %s.\n",
                    full_filename, strerror(errno));
            continue;
    	}
    	char * current_permissions = permission_string(&sb);
    	if (perm_int(current_permissions) ==  user_perm_to_int) {
      		printf("%s\n", full_filename);
    	}
    	free(current_permissions);
    	if (entry->d_type == DT_DIR) {
      		nav_tree(full_filename);
    	}
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[])
{
    if (argc == 1)
    {
        fprintf(stderr, "Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int d_flag = 0;
    int p_flag = 0;
    char opt;
    while ((opt = getopt(argc, argv, ":d:p:h")) != -1)
    {
        switch (opt)
        {
        case 'd':
            d_flag = 1;
            user_dir = optarg;
            break;
        case 'p':
            p_flag = 1;
            user_perm = optarg;
            break;
        case 'h':
            printf("Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
            return EXIT_SUCCESS;
        case '?':
            fprintf(stderr, "Error: Unknown option '%c' received.", opt);
            return EXIT_FAILURE;
        }
    }

    if (d_flag == 0)
    {
        fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
        return EXIT_FAILURE;
    }

    if (p_flag == 0)
    {
        fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
        return EXIT_FAILURE;
    }

    char path[PATH_MAX];
    if (realpath(user_dir, path) == NULL)
    {
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", user_dir, strerror(errno));
        return EXIT_FAILURE;
    }

    if (!verify_string(user_perm))
    {
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", user_perm);
        return EXIT_FAILURE;
    }

    user_perm_to_int = perm_int(user_perm);
    return nav_tree(path);
}