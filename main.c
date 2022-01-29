/* Author: Madeline Jacques (jacquema@oregonstate.edu)
   CS 344 W 22 Assignment 3
   Due Date: 2/7/22
   Last Updated: 1/27/22
   Description: TODO */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CMD_LENGTH 2048

   /* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */

// Consider defining a struct in which you can store all the different elements 
// included in a command. Then as you parse a command, you can set the value of 
// members of a variable of this struct type.
// The general syntax of a command line is:
// command [arg1 arg2 ...] [< input_file] [> output_file] [&]
// …where items in square brackets are optional.
// Your shell must support command lines with a maximum length of 2048 characters, 
// and a maximum of 512 arguments.
struct cmd_elements
{
    // Array of pointers to strings
    char *cmd;
    char *userArgs[512];
    int numArgs;
    char *inputFile;
    char *outputFile;
    int background;     // 0 = no, 1 = yes

};


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
struct cmd_elements *getUserInput()
{
    // char userInput[] = "cmd arg1hello arg2$$xyz$$qwr";
    char userInput[MAX_CMD_LENGTH * sizeof(char)];
    char *u = userInput;
    size_t buflen = MAX_CMD_LENGTH;
    ssize_t charsRead;

    // Use the colon : symbol as a prompt for each command line. 
    printf(": ");

    // flush out the output buffers each time you print, as the text that you're 
    // outputting may not reach the screen until you do in this kind of interactive 
    // program. To do this, call fflush() immediately after each and every time 
    // you output text.
    fflush(stdout);

    // The general syntax of a command line is:
    // command [arg1 arg2 ...] [< input_file] [> output_file] [&]
    charsRead = getline(&u, &buflen, stdin);

    // // Check if getline was successful, it will return -1 if not
    if (charsRead == -1)
    {
        // TODO - decide how to properly handle this error
        printf("Could not read input, getline error\n");
        fflush(stdout);
        exit(0);
    }
    // Check for blank input (/n or /0 = 1 char)
    if (charsRead < 2)
    {
        // TODO - decide how to properly handle this error
        printf("Blank entry\n");
        fflush(stdout);
        exit(0);
    }

    // // getline leaves newline in input, remove this
    userInput[strlen(userInput)-1] = '\0';
    
    // Check if the first character is a #, this means line is a comment
    if (userInput[0] == '#')
    {
        // TODO - decide how to properly handle this error
        printf("Blank entry\n");
        fflush(stdout);
        exit(0);
    }

    // Create a command struct to store data about the command entered
    struct cmd_elements *userCmd = malloc(sizeof(struct cmd_elements));
    
    // Initialize all userArgs elements to NULL
    for (int i = 0; i < 512; i++)
    {
        userCmd->userArgs[i] = NULL;
    }

     // Extract the command elements
    char *saveptr;
    char *token = strtok_r(userInput, " ", &saveptr);
    userCmd->cmd = calloc(strlen(token) + 1, sizeof(char));   // The first token is the command
    strncpy(userCmd->cmd, token, (strlen(token) + 1));

    int argIndex = 0;

    // strtok_r returns a pointer to the next token or NULL if there are no more
    while (token != NULL)
    {
        token = strtok_r(NULL, " ", &saveptr); // this advances to the next token

        if (token != NULL)
        {
            // check if the token is < > or &
            if (strcmp(token, "<") == 0)
            {
                // then the next token we would want to store in userCmd->inputFile
                // advance past the "<" to get it
                token = strtok_r(NULL, " ", &saveptr);
                userCmd->inputFile = calloc(strlen(token) + 1, sizeof(char));
                strncpy(userCmd->inputFile, token, (strlen(token) + 1));
            }
            else if (strcmp(token, ">") == 0)
            {
                // then the next token we would want to store in userCmd->outputFile
                // advance past the ">" to get it
                token = strtok_r(NULL, " ", &saveptr);
                userCmd->outputFile = calloc(strlen(token) + 1, sizeof(char));
                strncpy(userCmd->outputFile, token, (strlen(token) + 1));
            }
            // start storing arguments
            else
            {
                // Check token for $$ variables
                char newStr[100];
                int pid = getpid();
                char pidStr[50];
                snprintf(pidStr, sizeof(pidStr), "%d", pid);
                int tokenLen = strlen(token);
                int nextIndexNewStr = 0;
                //int nextIndexToken = 0;

                // Initialize newStr
                for (int n = 0; n < 100; n++)
                {
                    newStr[n] = '\0';
                }

                for (int i = 0; i < tokenLen; i++)
                {
                    if (token[i] == '$' && token[i + 1] == '$')
                    {
                        // at next available index of newStr, start writing the variable
                        for (int j = 0; j < strlen(pidStr); j++)
                        {
                            newStr[nextIndexNewStr] = pidStr[j];
                            nextIndexNewStr++;
                        }
                        // skip the next char in the token
                        i++;

                    }
                    // Next char is not a $ followed by another $ and should be copied over
                    else
                    {
                        newStr[nextIndexNewStr] = token[i];
                        nextIndexNewStr++;
                    }
                }

                char *heapPtr = calloc(strlen(newStr) + 1, sizeof(char));
                strncpy(heapPtr, newStr, (strlen(newStr) + 1));
                userCmd->userArgs[argIndex] = heapPtr;
                argIndex++;
                userCmd->numArgs++;
            }
        }
    }

    // Check if the last argument is "&", set background flag if so
    if (strcmp(userCmd->userArgs[userCmd->numArgs - 1], "&") == 0)
    {
        userCmd->background = 1;
        // This also cuts off any args entered after & which should not be
        // considered valid anyways.
        userCmd->userArgs[userCmd->numArgs - 1] = NULL;
        free(userCmd->userArgs[userCmd->numArgs - 1]);
        userCmd->numArgs--;
        argIndex--;
    }

    return userCmd;

}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void exitCmd()
{
    // TODO - make sure this kills all child processes of the parent
    exit(0);
}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void chDirCmd(struct cmd_elements *currentCmd)
{    
    // If there is no argument, take the user to the home directory
    if (currentCmd->numArgs == 0)
    {
        // The function getenv looks for the environment variable name. 
        // If this variable is defined, the function returns a pointer to its value. 
        // If this variable is not in the environment list, then NULL is returned.
        chdir(getenv("HOME"));
    }
    // Is the path in the form /xxxx? Then it is an absolute path.
    else if (currentCmd->userArgs[0][0] == '/')
    {
        if (chdir(currentCmd->userArgs[0]) == -1)
        {
            printf("cd error - filepath %s is not valid.\n", currentCmd->userArgs[0]);
            fflush(stdout);
        }
        else
        {
            chdir(currentCmd->userArgs[0]);
        }
    }
    // Otherwise, the path is in the form subfolder/etc
    // and is a relative path, so we need to append it to the cwd.
    else
    {
        char targetDir[MAX_CMD_LENGTH];
        getcwd(targetDir, sizeof(targetDir));
        strcat(targetDir, "/");
        strcat(targetDir, currentCmd->userArgs[0]);

        if (chdir(targetDir) == -1)
        {
            printf("cd error - filepath %s is not valid.\n", targetDir);
            fflush(stdout);
        }
        else
        {
            chdir(targetDir);
        }
    }
    // The getcwd() function copies an absolute pathname of the current
    //    working directory to the array pointed to by buf, which is of
    //    length size. (https://man7.org/linux/man-pages/man3/getcwd.3.html)


}

// Debug printing
void printCommand(struct cmd_elements *currentCmd)
{
    printf("Command Entered: %s\n", currentCmd->cmd);
    fflush(stdout);
    int argIndex = 0;
    while (currentCmd->userArgs[argIndex] != NULL)
    {
        printf("Arg %d: %s\n", argIndex + 1, currentCmd->userArgs[argIndex]);
        fflush(stdout);
        argIndex++;
    }
    printf("Input redirect: %s\n", currentCmd->inputFile);
    fflush(stdout);
    printf("Output redirect: %s\n", currentCmd->outputFile);
    fflush(stdout);
    printf("NumArgs: %d\n", currentCmd->numArgs);
    fflush(stdout);
    printf("Background (0 for no, 1 for yes): %d\n", currentCmd->background);
    fflush(stdout);
}


int main(void)
{
    int continueProgram = 1;

    while (continueProgram < 5)
    {
    
    struct cmd_elements *currentCmd = getUserInput();
    
    // Debug printing
    printCommand(currentCmd);

    if (strcmp(currentCmd->cmd, "exit") == 0)
    {
        exitCmd();
    }

    if (strcmp(currentCmd->cmd, "cd") == 0)
    {
        char buffer[100];
        int bufsize = 100;
        char buffer2[100];
        int bufsize2 = 100;

        getcwd(buffer, bufsize);
        printf("cwd before calling chDirCmd: %s\n", buffer);
        chDirCmd(currentCmd);
        getcwd(buffer2, bufsize2);
        printf("cwd after calling chDirCmd: %s\n", buffer2);
        // /Users/mjacq/CS344/
    }

    continueProgram++;
    }

    return 0;
}