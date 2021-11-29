#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <grp.h>
#include <sys/stat.h>

#define _XOPEN_SOURCE  500
#define _POSIX_C_SOURCE 200112L
#define MAX_SIZE 1024
#define SUCCESS 1
#define FAIL 0

//This function prints inode number without long format
void print_without_long(struct dirent *file_n, const char *file_path, int input1)
{
    if (input1 == 1)
        printf("%lu ", file_n->d_ino);
    printf("%s\n", file_n->d_name);
}

//this function lists the file paths for i-node number and recursive files and in long format
void list_paths(const char *filep, void (*list_file)(struct dirent *, const char *, int), int input1, int input2)
{
    int ptr1;
    int ptr2;
    if (input1 == -1 && input2 == -1)
    {
        perror("the arguments are not passed correctly\n");
        return;
    }

    struct dirent *input;
    DIR *dir;

    if (!(dir = opendir(filep)))
    {
        perror("this directory does not exist\n");
        return;
    }
    if (filep == NULL)
    {
        perror("the directory is empty");
        return;
    }
    // if the input command given is -i this prints file with i-node number inn long format
    while ((input = readdir(dir)) != NULL)
    {
        if (input->d_type == DT_REG)
        {
            if (input->d_name[0] == '.'){
                ptr1 = 0;
                ptr2 = 1;
                continue;
            }

            list_file(input, filep, input1);
        }
        else if (input->d_type == DT_DIR)
        {
            if (input->d_name[0] == '.'){
                ptr1 = 1;
                ptr2 = 0;
                continue;
             }
            list_file(input, filep, input1);
        }
        else if (input->d_type == DT_LNK)
        {
            if (input->d_name[0] == '.')
                continue;

            list_file(input, filep, input1);
        }
    }
    closedir(dir);
    
    if(ptr1 == ptr2){
        perror("this directory can not be accessed\n");
        return;
    }

    if (!(dir = opendir(filep)))
    {
        perror("the directory does not exist\n");
        return;
    }

    // this prints recursive files.(-R)
    // learnt from:
    // https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux

    if (input2 == 1)
    {
        
        while ((input = readdir(dir)) != NULL)
        {
            if (input->d_type == DT_DIR)
            {
                char buff[MAX_SIZE];
                char *file_path;
                size_t MAXLT = 256;
                file_path = malloc(MAXLT + 1);

                if (input->d_name[0] == '.')
                    continue;
                snprintf(buff, sizeof(buff), "%s/%s", filep, input->d_name);
                printf("\n\n%s:\n", buff);
                list_paths(buff, list_file, input1, input2);
            }
        }
    }
    closedir(dir);
}

//checks the arguments passed
int arg_checking(int input1, int input2)
{
    if (input1 == input2)
        return SUCCESS;
    if (input1 == -1 && input2 == -1)
        return FAIL;
}

//this function prints file in long and if the i node number is given then it prints with i node
void print_long_path(struct dirent *file_n, const char *input_path, int cmd_input)
{
    int ptr1 = -1;
    int ptr2 = -2;
    char file_p[MAX_SIZE];
    char date_display[20];
    struct stat sb;

    snprintf(file_p, sizeof(file_p), "%s/%s", input_path, file_n->d_name);

    lstat(file_p, &sb);
    struct group *group = NULL;
    group = getgrgid(sb.st_gid);
    struct passwd *pwd = NULL;
    pwd = getpwuid(sb.st_uid);

    if (cmd_input == 1)
    {
        printf("%10lu", sb.st_ino);
        printf("\t\t ");
        ptr1 = 1;
        ptr2 = 0;
    }
    
    if(ptr1 == ptr2){
        perror("arguments error");
        return;
    }

    struct tm *time = NULL;

    //using mtime to avoid single string
    time = localtime(&sb.st_mtime);

    // printing the permissions
    //From: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
    printf((S_ISDIR(sb.st_mode)) ? "d" : "-");
    printf((sb.st_mode & S_IRUSR) ? "r" : "-");
    printf((sb.st_mode & S_IWUSR) ? "w" : "-");
    printf((sb.st_mode & S_IXUSR) ? "x" : "-");
    printf((sb.st_mode & S_IRGRP) ? "r" : "-");
    printf((sb.st_mode & S_IWGRP) ? "w" : "-");
    printf((sb.st_mode & S_IXGRP) ? "x" : "-");
    printf((sb.st_mode & S_IROTH) ? "r" : "-");
    printf((sb.st_mode & S_IWOTH) ? "w" : "-");
    printf((sb.st_mode & S_IXOTH) ? "x" : "-");

    if (cmd_input == 1)
        printf("\t\t");

    printf("%4lu", sb.st_nlink);
    printf("%10s ", pwd->pw_name);
    printf("%8s", group->gr_name);
    printf("%8lu", sb.st_size);

    //time display
    localtime_r(&(sb.st_mtime), &time);

    //formatting date and time for the files
    const char *date_format = "%b %d %H:%M";
    strftime(date_display, 20, date_format, &time);
    printf(" %s ", date_display);
    printf("%s", file_n->d_name);

    char *temp_path;
    char p;
    size_t MAXL = 1024;
    ssize_t len;
    
    if (file_n->d_type == DT_LNK)
    {
        len = readlink(input_path, temp_path, MAXL);
        printf(" -> %s", file_p, temp_path);
        ptr1 = 0;
        ptr2 =1;
        
    }

    printf("\n");


    if(ptr1 == ptr2){
        perror("arguments error");
        return;
    }
}

int main(int argc, char **argv)
{

    int input_l = 0;
    int input_i = 0;
    int input_R = 0;
    int argument = 1;

    // with only./UnixLs it prints the current path files
    if (argc == 1)
    {
        list_paths(".", print_without_long, input_i, input_R);
        printf("\n");
        printf("\n");
        return 0;
    }

    int input;
    while ((input = getopt(argc, argv, "iRl")) != -1)
    {
        switch (input)
        {
        case 'i':
            input_i = 1;
            break;
        case 'l':
            input_l = 1;
            break;
        case 'R':
            input_R = 1;
            break;
        default:
            printf("failed");
            return -1;
        }
    }

    //for checking the ".", ".."and"~"

    int a;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ".") == 0)
        {
            a = i;
            break;
        }
        else
            a = -1;
    }
    int b;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "..") == 0)
        {
            b = i;
            break;
        }
        else
            b = -1;
    }
    int c;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "~") == 0)
        {
            c = i;
            break;
        }
        else
            c = -1;
    }

    if (argv[argc - 1][0] == '-')
        argument = 0;

    if (argument == 1)
    {
        if (input_l == 0)
        {
            for (int i = optind; i < argc; i++)
            {
                printf("%s:\n", argv[i]);
                list_paths(argv[i], print_without_long, input_i, input_R);
                printf("\n\n");
            }
            printf("\n");
            return 0;
        }
        else
        {
            for (int i = optind; i < argc; i++)
            {
                printf("%s:\n", argv[i]);
                list_paths(argv[i], print_long_path, input_i, input_R);
                printf("\n\n");
            }
            return 0;
        }
    }

    if (input_l == 0)
    {

        if (a != -1)
            list_paths(".", print_without_long, input_i, input_R);
        if (b != -1)
            list_paths("..", print_without_long, input_i, input_R);
        if (c != -1)
            list_paths("~", print_without_long, input_i, input_R);
        else
            list_paths(".", print_without_long, input_i, input_R);
        printf("\n");
        return 0;
    }
    if (input_l == 1)
    {
        if (a != -1)
            list_paths(".", print_long_path, input_i, input_R);
        if (b != -1)
            list_paths("..", print_long_path, input_i, input_R);
        if (c != -1)
            list_paths("~", print_long_path, input_i, input_R);
        else
            list_paths(".", print_long_path, input_i, input_R);
    }

    return 0;
}
