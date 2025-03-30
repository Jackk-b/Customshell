#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> //libaries included
#include "customshell.h"

/*Function changes the directory and updates PWD if arguements are specified,
otherwise prints out current directory*/
void change_directory(char **args)
{
    if (args[1] == NULL) // If no args are present do the following
    {
        char cwd[MAX_BUFFER];
        if (getcwd(cwd, sizeof(cwd)) != NULL) // Get the current directory
        {
            printf("Current working directory is: %s\n", cwd); // Print the current directory
        }
        else // Error handling if getcwd fails
        {
            perror("Error printing current directory");
        }
    }
    else // If args are present then change the directory
    {
        if (chdir(args[1]) == 0) // Change the directory to the one specified
        {
            setenv("PWD", args[1], 1); // Change the PWD varaible
        }
        else // Error handling for an error in changing directory
        {
            perror("Error changing diectory");
        }
    }
}

/*Function clears the screen*/
void clear()
{
    system("clear"); // Using system function
}

/*Function lists the contents of the directory*/
void dir()
{
    pid_t pid; // Define pid
    int status;
    bool dont_wait = false;

    char *cmd = "ls"; // Command to be executed
    char *argv[3];
    argv[0] = "ls";
    argv[1] = "-la"; // Options with command
    argv[2] = NULL;

    switch (pid = fork()) // Fork process
    {
    case -1:
        perror("fork");
        return;
    case 0:                // Child process
        execvp(cmd, argv); // Execute command
        perror("exec");
    default:
        if (!dont_wait) // Waiting for child
            waitpid(pid, &status, WUNTRACED);
    }
}

/*Function prints out environment varaibles*/
void environment_varibles()
{
    extern char **environ;                   // Get the external varaible environ
    for (int i = 0; environ[i] != NULL; ++i) // For loop to print out each one until null value is encountered
    {
        printf("%s\n", environ[i]); // Print each one
    }
}

/*Funtion echos the args*/
void echo(char **args)
{
    char **arg = args;
    ++arg; // Increment once so command isnt shown
    while (*arg)
    {                        // While NULL isnt encounterd
        printf("%s ", *arg); // Print args
        ++arg;               // Increment each time
    }
    fputs("\n", stdout); // Print a newline
}

/*Function pauses the shell, user presses enter key to continue*/
void pause_shell()
{
    printf("Press Enter to continue...");
    getchar(); // Waits for enter
}

/*Function shows manual.txt, a help page*/
void help()
{
    char *filename = "../manual/manual.txt";
    FILE *manual = fopen(filename, "r"); // Open manual
    if (!manual)
    {
        printf("Sorry, error opening manual");
    }

    pid_t pid;
    int status;
    bool dont_wait = false;

    char *cmd = "more"; // Command to execute
    char *argv[3];
    argv[0] = "more";
    argv[1] = "../manual/manual.txt";
    argv[2] = NULL;

    switch (pid = fork())
    {
    case -1:
        perror("fork");
        fclose(manual); // Close manual once done
        return;
    case 0: // child
        execvp(cmd, argv);
        perror("exec");
        fclose(manual); // Close the manual
    default:
        if (!dont_wait)
            waitpid(pid, &status, WUNTRACED);
    }
}

/*Function quits the program*/
void quit()
{
    exit(0); // Use exit to break out of the while loop
}

/*Function uses fork and exec to handle external commands*/
void external_command(char **args)
{
    pid_t pid;
    int status;
    bool dont_wait = false;
    FILE *pfile = NULL;

    for (int i = 0; args[i] != NULL; ++i)
    { // Loop through args

        if (strcmp(args[i], "&") == 0)
        { // If the & symbol is present, then update value of dont_wait variable
            dont_wait = true;
            args[i] = NULL; // Remove the & symbol from args
        }

        if (strcmp(args[i], "<") == 0)
        { // Check for input symbol
            if (args[i + 1] == NULL)
            { // Check if there is input present
                perror("No input file detected");
                return;
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], R_OK) == -1))
            { // Check is file exists and that it has the correct permissions
                perror("Input file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "r", stdin); // Open the file as write
            if (!pfile)
            { // Check file opened
                perror("Failed to open file");
                return;
            }
            args[i] = NULL;     // Remove symbol from args
            args[i + 1] = NULL; // Remove filename from args
        }

        else if (strcmp(args[i], ">") == 0)
        { // Check for output symbol
            if (args[i + 1] == NULL)
            { // Check if input is presesnt
                perror("No input file detected");
                return;
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], W_OK) == -1))
            { // Check if file exists and that it has the correct permisions
                perror("Output file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "w", stdout);
            if (!pfile)
            { // Check file opened
                perror("Failed to open file");
                return;
            }
            args[i] = NULL; // Remove symbol and filename from args
            args[i + 1] = NULL;
        }

        else if (strcmp(args[i], ">>") == 0)
        { // Check for output symbol
            if (args[i + 1] == NULL)
            { // Check if input is present
                perror("No input file detected");
                return;
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], W_OK) == -1))
            { // Check if input file exists and that is has the correct permissions
                perror("Output file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "a", stdout);
            if (!pfile)
            { // Check file opened
                perror("Failed to open file");
                return;
            }
            args[i] = NULL; // Remove symbol and filename from args
            args[i + 1] = NULL;
        }
    }

    switch (pid = fork()) // Use switch instead of multiple if statements
    {
    case -1: // Error
        perror("fork");
        return;

    case 0:                                   // Child process
        setenv("PARENT", getenv("shell"), 1); // Set environment varaible
        execvp(args[0], args);                // Execute command
        perror("exec");                       // If we get to here there has been an error
        return;

    default:
        if (!dont_wait)
        { // Define wether to wait for the child process
            waitpid(pid, &status, WUNTRACED);
        }
    }
}