#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>     //Include Libraries
#include "customshell.h" //Include functions

int main(int argc, char **argv)
{
    char buf[MAX_BUFFER];        // Line buffer
    char *args[MAX_ARGS];        // Pointers to arguement strings
    char **arg;                  // Working pointer thru args
    char *prompt = "==>";        // Prompt without current directory name
    char dir_prompt[MAX_BUFFER]; // Prompt with current directory name

    char path[MAX_BUFFER];                              // Buffer for path
    readlink("/proc/self/exe", path, sizeof(path) - 1); // Get path
    setenv("shell", path, 1);                           // Set shell envorinment varaible

    FILE *input = stdin; // Input pointer to stdin

    if (argc == 2)
    { // If a filename is supplied
        char *filename = argv[1];
        FILE *input = fopen(filename, "r"); // Open the file and assign it to input
        if (!input)
        { // Check that file was sucessful opening
            printf("Failed to open %s\n", filename);
        }
    }

    while (!feof(input))
    { // Keep reading till EOF or quit internal command is called
        if (input == stdin)
        { // Only show prompt if no batchfile was given
            if (getcwd(dir_prompt, sizeof(dir_prompt)) != NULL)
            {                                // Get current directory for prompt
                strcat(dir_prompt, " ==> "); // Append the prompt to the directory name
                fputs(dir_prompt, stdout);   // Write directory name and prompt to stdout
            }
            else
            {                          // If getting the current directory fails
                fputs(prompt, stdout); // Write simple prompt
            }
        }

        if (fgets(buf, MAX_BUFFER, input))
        { // Read line from input
            arg = args;
            *arg++ = strtok(buf, SEPARATORS); // Tokenise the input

            while ((*arg++ = strtok(NULL, SEPARATORS)))
                ;

            if (args[0])
            {                          // If there's any input
                process_command(args); // Process the command
            }
        }
    }
    return 0;
}

/*Function checks for internal command and processes it or passes it on if its an external command,
  also handles file redirection*/
void process_command(char **args)
{

    int original_stdin = dup(STDIN_FILENO); // Stores original stdin/stdout
    int original_stdout = dup(STDOUT_FILENO);

    FILE *pfile; // Initalize file pointer

    if (args[0] == NULL)
    { // If there are no args, return from function
        return;
    }

    for (int i = 0; args[i] != NULL; ++i)
    { // Loop through and check for file redirection symbols

        if (strcmp(args[i], "<") == 0)
        { // Check for input symbol
            if (args[i + 1] == NULL)
            { // Check if there is any file present
                perror("No input file detected");
                return; // Return function
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], R_OK) == -1))
            { // Check that file exists and has read permissions
                perror("Input file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "r", stdin); // Open file to read as stdin
            if (!pfile)
            { // If failed to open then return
                perror("Failed to open file");
                return;
            }
            args[i] = NULL;     // Replace the redirection symbol with null
            args[i + 1] = NULL; // Replace the file name with null
        }

        else if (strcmp(args[i], ">") == 0)
        { // Check for output symbol
            if (args[i + 1] == NULL)
            { // Check for file
                perror("No input file detected");
                return;
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], W_OK) == -1))
            { // Check file exixts and permissions
                perror("Output file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "w", stdout); // Open file as write for stdout
            if (!pfile)
            { // Check file opened
                perror("Failed to open file");
                return;
            }
            args[i] = NULL; // Replace symbol and file name
            args[i + 1] = NULL;
        }

        else if (strcmp(args[i], ">>") == 0)
        { // Check for output symbol (append to output file)
            if (args[i + 1] == NULL)
            { // Check for file
                perror("No input file detected");
                return;
            }
            if ((access(args[i + 1], F_OK) == -1) || (access(args[i + 1], W_OK) == -1))
            { // Check file exists and permissions
                perror("Output file doesnt exist or has incorrect permissions!");
                return;
            }
            pfile = freopen(args[i + 1], "a", stdout); // Open file as append for stdout
            if (!pfile)
            { // Check file opened
                perror("Failed to open file");
                return;
            }
            args[i] = NULL; // Replace symbol and file name
            args[i + 1] = NULL;
        }
    }

    Command commands[] = {
        // All internal commands
        {"cd", change_directory},
        {"clr", clear},
        {"dir", dir},
        {"environ", environment_varibles},
        {"echo", echo},
        {"pause", pause_shell},
        {"help", help},
        {"quit", quit},
    };

    int num_commands = 8; // Number of internal commands, needed for loop

    /*Loop loops through commands, comparing args to each one to see if an
    internal command has been supplied*/
    for (int i = 0; i < num_commands; ++i)
    {
        if (strcmp(args[0], commands[i].command) == 0)
        {
            commands[i].func(args);

            dup2(original_stdin, STDIN_FILENO);
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdin);
            close(original_stdout);

            return; // Returns from function if command found
        }
    }

    /*For external commands we will use fork and exec to execute them*/
    external_command(args);

    dup2(original_stdin, STDIN_FILENO); // Reinstate standard input/output from above
    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdin); // Close the files
    close(original_stdout);
}