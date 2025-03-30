#include <stdio.h>

#define MAX_BUFFER 1024    // max line buffer
#define MAX_ARGS 64        // max # args
#define SEPARATORS " \t\n" // token separators

void process_command(char **args);  // Functions to process commands that come in
void external_command(char **args); // Function handle external commands

void change_directory(char **args); // Function definitons
void clear();
void dir();
void environment_varibles();
void echo(char **args);
void help();
void pause_shell();
void quit();

typedef struct Command Command; // Struct to handle all the internal commands
struct Command
{
    char *command;
    void (*func)(char **args);
};