/* CS 314
 * Gavin Witsken, Timothy Hemphill
 * pish
 * 10/23/2022
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "environment.h"
#include "command_struct.h"

env_var* env;
bool do_exit = false; //Determines when to exit input prompt loop
int saved_stdout;
int saved_stdin;

char** tokenificate(char* input, int* arg_count); //Split command into tokens (array of arguments)
char* get_input();
bool command_listificate(command_list* cmd_list, char** list, int arg_count); //create a command list struct
bool is_separator(char* c);
void execute(command_struct* command); //Execute a command, modifies do_exit to true for built-in command exit
bool redir_input(command_struct* command); //Redirects input to file
bool redir_output(command_struct* command); //Redirects output to (and overwrites) file
bool redir_output_append(command_struct* command); //Redirects output and appends to file
void make_pipe(command_struct* command); //Makes a pipe between processes

void reset_io();
char* simplify_path(char* fullpath);
char* get_args_string(command_struct* command);
void load_pishrc();

int main(int argc, char** argv)
{
    //Do signal handling
    //signal(SIGQUIT, SIG_IGN); //Ignore quit (ctr + \) signal
	signal(SIGINT, SIG_IGN); //Ignore interupt (ctr + C) signal
	signal(SIGTSTP, SIG_IGN); //Ignore terminal stop (ctr + Z) signal
    signal(SIGTERM, SIG_IGN); //Ignore Termination signals

    //Set initial environment variables
    env = create_env_var("PATH", getenv("PATH")); //Get initial variable, establishes linked list
    env = add_env_var(env, "DIR", getenv("PWD")); //Add current working dir
    char homeBuff[1024];
    strcpy(homeBuff, getenv("PWD"));
    env = add_env_var(env, "HOME", homeBuff); //Add home
    // env = add_env_var(env, "TEST", "testing removing root"); //Add home
    saved_stdin = dup(0);
    saved_stdout = dup(1);

    //Load stuff from .pishrc
    load_pishrc();

    char* input;

    printf("Welcome to pish!\n\n");

    while (!do_exit) {
        int arg_count = 0;
        command_list cmd_list;
        command_list_init(&cmd_list);

        reset_io();
        input = get_input(); //Get input from command line
        
        char** token_list = tokenificate(input, &arg_count); //Make input into tokens

        if (!command_listificate(&cmd_list, token_list, arg_count)) { //Listification failed :(
            fprintf(stderr, "Error: unable to create list from provided arguments\n");
        } else {
            // command_list_print(&cmd_list);

            command_struct* command = cmd_list.root;

            while (command != NULL) {
                // fprintf(stderr, "Checking for separators\n");
                if (command->right_separator != NULL) {
                    if (strcmp(command->right_separator, "<") == 0) {
                        // fprintf(stderr, "Redirecting input\n");
                        redir_input(command->next);
                    } else if (strcmp(command->right_separator, ">") == 0) {
                        // fprintf(stderr, "Redirecting output\n");
                        redir_output(command->next);
                    } else if (strcmp(command->right_separator, ">>") == 0) {
                        // fprintf(stderr, "Redirecting output (append)\n");
                        redir_output_append(command->next);
                    } else if (strcmp(command->right_separator, "|") == 0) {
                        // fprintf(stderr, "Making pipe\n");
                        make_pipe(command);
                    } else {
                        //Maybe something goes here?
                    }
                }
                
                // fprintf(stderr, "Trying to fetch value for left separator of command\n");
                char* last_separator = command->left_separator;
                // fprintf(stderr, "Successfully fetched value for left separator of command: %s\n", last_separator);

                if (!is_separator(last_separator)) {
                    if (command->right_separator != NULL) {
                        if (strcmp(command->right_separator, "|") != 0) {
                            execute(command);
                        }
                    } else {
                        execute(command); 
                    }
                }

                command = command->next;
                
            }
        }
        // do_exit = true;

        // fprintf(stderr, "Deleting command list\n");
        command_list_delete(&cmd_list);
    }

    return 0;
}

char** tokenificate(char* input, int* arg_count) {

    *arg_count = 0;
    char* list[1000];

    char* token = strtok(input, " ");

    while (token != NULL) {
        if(strcmp(token, " ") != 0 && strcmp(token, "\n") != 0) {
            list[*arg_count] = token;
            ++(*arg_count);
        }

        token = strtok(NULL, " ");
    }

    const char* word = list[*arg_count - 1];
    int len = strlen(word);
    list[*arg_count - 1][len-1] = '\0'; //Take out newline character form last argument


    char** cmd_list = NULL;

    if ((*arg_count) != 0) { //allocate space for array only if something other than whitespace was found
        cmd_list = malloc( (*arg_count) * sizeof(char*));
    }

    for(int i = 0; i < *arg_count; ++i) {
        cmd_list[i] = list[i];
    }

    return cmd_list;
}

char* get_input() {
    char* buff;
    size_t buffsize = 100;
    size_t line;

    buff = malloc(buffsize * sizeof(char));

    if (buff == NULL) { //malloc failed
        fprintf(stderr, "Error: unable to allocate memory for input buffer\n");
        exit(1);
    }
    char* temp_path = search_env_var(env, "DIR");

    printf("pish%s %s> ", "%", simplify_path(temp_path));
    line = getline(&buff, &buffsize, stdin); //Automatically resizes buffer if needed

    return buff;
}

bool command_listificate(command_list* cmd_list, char** list, int arg_count) {

    int sep_count = 0;
    int* separators;

    if (arg_count <= 0) {
        fprintf(stderr, "Error: not enough arguments");
        return false;
    }

    if (is_separator(list[arg_count-1])) {
        fprintf(stderr, "Error: %s cannot be the final argument\n", list[arg_count-1]);
        return false;
    }

    if (is_separator(list[0])) {
        fprintf(stderr, "Error: %s cannot be the first argument\n", list[0]);
        return false;
    }

    for (int i = 0; i < arg_count - 1; i++) {
        if (is_separator(list[i]) && is_separator(list[i+1])) {
            fprintf(stderr, "Error: cannot have two consecutive separators: %s and %s \n", list[i], list[i-1]);
            return false;
        }
    }

    for (int i = 0; i < arg_count; i++) { //Make list of separators  
        if (is_separator(list[i])) {
            if (sep_count == 0) {
                separators = malloc(sizeof(int));
                separators[sep_count] = i;
                sep_count++;
            } else {
                separators = realloc(separators, (sep_count+1) * sizeof(int*));
                separators[sep_count] = i;
                sep_count++;
            }
        }
    }

    int last_sep = -1;
    int command_count = sep_count+1;
    command_struct* commands = malloc(command_count*sizeof(command_struct));

    for (int i = 0; i < command_count - 1; i++) { //Insert all but last thing in there
        command_struct_init(&(commands[i]));

        if (last_sep == -1) { //is first command
            commands[i].right_separator = list[separators[i]];
        } else {
            commands[i].left_separator = list[separators[i-1]];
            commands[i].right_separator = list[separators[i]];
        }


        for (int j = last_sep + 1; j < separators[i]; j++) {
            command_struct_arg_insert(&(commands[i]), list[j]);
            last_sep = j + 1;
        }
    }

    command_struct_init(&(commands[command_count-1])); //Insert last thing in there
    if (sep_count > 0) {
        commands[command_count-1].left_separator = list[last_sep];
    }
    for (int i = last_sep + 1; i < arg_count; i++) {
        command_struct_arg_insert(&(commands[command_count-1]), list[i]);
    }

    command_struct_arg_insert(&(commands[command_count-1]), NULL); //Add Null delimiter for process

    for (int i = 0; i < command_count; i++) { //Insert commands into command list

        commands[i].path = commands[i].args[0];
        command_list_insert(cmd_list, &(commands[i]));
    }

    if (sep_count > 0) {
        free(separators);
    }

    return true;
}

bool is_separator(char* c) {
    if (c == NULL) {
        return false;
    }
    else if ( (strcmp(c, "|") == 0) || (strcmp(c, "<") == 0) || (strcmp(c, ">") == 0) || (strcmp(c, ">>") == 0)) { 
        return true;
    }
    //else 
        return false;
}

bool redir_input(command_struct* command) {
    int input_fd = open(command->path, O_RDONLY); //Open file for reading
    if (input_fd == -1) {
        fprintf(stderr, "Could not open file %s for reading\n", command->path);
        return false;
    }

    dup2(input_fd, 0); //Map stdin to input_fd
    close(input_fd); //Close file

    return true;
}
bool redir_output(command_struct* command) {
    int output_fd = open(command->path, O_WRONLY | O_CREAT | O_TRUNC, 0600);//open file for writing, create if does not exist (read/write for user), first truncate to 0 before anything is written
    if (output_fd == -1) {
        fprintf(stderr, "Could not open file %s for writing\n", command->path);
        return false;
    }

    // fprintf(stderr, "Opened file %s for writing\n", command->path);

    dup2(output_fd, 1); //Map stdout to output_fd
    close(output_fd); //Close file

    return true;
}
bool redir_output_append(command_struct* command) {
    int output_fd = open(command->path, O_WRONLY | O_CREAT | O_APPEND, 0600);//open file for writing, create if does not exist (read/write for user), first truncate to 0 before anything is written
    if (output_fd == -1) {
        fprintf(stderr, "Could not open file %s for writing\n", command->path);
        return false;
    }

    dup2(output_fd, 1); //Map stdout to output_fd
    close(output_fd); //Close file

    return true;
} //Redirects output and appends to file
void make_pipe(command_struct* command) {
    int fd[2];
    pipe(fd); //create pipe to suck stuff up

    dup2(fd[1], 1); //Map stdout of command to write end of pipe
    close(fd[1]); //Close write end of pipe - no leaks

    // fprintf(stderr, "Command:\n");
    // for (int i = 0; i < command->arg_count; i++) {
    //     fprintf(stderr, "%d: %s\n", i, command->args[i]);
    // }

    execute(command); //Run command

    dup2(fd[0], 0); //Map stdin for next process to read end of pipe
    close(fd[0]); //Close read end of pipe - no leaks

    execute(command->next);
}

void execute(command_struct* command) {

    // fprintf(stderr, "Executing command %s\n", command->path);
    int pid;

    if (strcmp(command->path, "exit") == 0) {
        printf("Exiting pish\n");
        do_exit = true;
    } else if (strcmp(command->path, "cd") == 0) {    
        if (command->arg_count != 2 && command->arg_count != 1) {
            fprintf(stderr, "Error: Invalid use of cd\n");
        } else if (command->arg_count == 2){
            char* test_char = strrchr(command->args[1], '/');
            //printf("%s\n", test_char);
            //search second arg for a slash then do something
            if (test_char == NULL){  
                //printf("Entering cd for appending to a path");          
                char* curr_dir = search_env_var(env, "DIR");
                strcat(curr_dir, "/");
                strcat(curr_dir, command->args[1]);
                //fprintf(stderr, "Trying to change to %s\n", curr_dir);
                if (chdir(curr_dir) == 0){
                    env = add_env_var(env, "DIR", curr_dir);
                }
                else{
                    fprintf(stderr, "Error: could not change directories\n");
                }
            }             
        }
        else{
            char* home_dir = search_env_var(env, "HOME");
            if (chdir(home_dir) == 0){
                    env = add_env_var(env, "DIR", home_dir);
                }
                else{
                    fprintf(stderr, "Error: could not change directories\n");
                }
        }

    } else if (strcmp(command->path, "setenv") == 0) {
        if (command->arg_count != 1 && command->arg_count != 3) {
            fprintf(stderr, "Error: Invalid use of setenv\n");
        }
        // command -> args : ex: setenv variable varValue
        else if (command->arg_count == 1){
            print_env(env);
        }
        else{
           env = add_env_var(env, command->args[1], command->args[2]);
        }

    } else if (strcmp(command->path, "unsetenv") == 0) {
        if (command->arg_count != 2) {
            fprintf(stderr, "Error: Invalid use of unsetenv\n");
        }
        else{
            env = delete_env_var(env, NULL, command->args[1]);
        }
        
    } else { //Not a built-in, fork and exec
        pid = fork();
        if (pid < 0) { //Fork failed
            fprintf(stderr, "Error: could not fork\n");
        } else if (pid == 0) { //Child
            execvp(command->path, command->args);  
            //if here, then exec failed
            fprintf(stderr, "Error: could not execute command %s\n", command->path);
            exit(1);
        } else { //Parent
            wait(NULL);
        }
    }
    reset_io();
}


void reset_io() {
    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
}

char* simplify_path(char* fullpath) {
    char* pathcopy = fullpath;

    pathcopy = strrchr(pathcopy, '/');

    return pathcopy + 1;
}

char* get_args_string(command_struct* command) {
    char* str = malloc (sizeof(command->args));
    int count = 0;

    for (int i = 0; i < command->arg_count; i++) {
        for (int j = 0; j < strlen(command->args[j]); j++) {
            str[count] = command->args[i][j];
            ++count;
        }
    }

    str[count] = '0';

    return str;
}

void load_pishrc() {
    FILE* fp = fopen("./.pishrc", "r");

    if (fp != NULL) {
        char* line = NULL;
        size_t length = 0;

        while(getline(&line, &length, fp) != -1) {
            system(line);
        }

        free(line);
    } else {
        fprintf(stderr, "Could not open file .pishrc in current directory for reading\n");
    }
}