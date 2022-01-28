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

    // Extract the command elements
    struct cmd_elements *userCmd = malloc(sizeof(struct cmd_elements));
    char *saveptr;
    // The first token is the command
    // strtok_r returns a pointer to the next token or NULL if there are no more
    char *token = strtok_r(userInput, " ", &saveptr);
    userCmd->cmd = calloc(strlen(token) + 1, sizeof(char));
    strncpy(userCmd->cmd, token, (strlen(token) + 1));

    int argIndex = 0;
    
    while (token != NULL)
    {
        printf("%s\n", token); // this is the command
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
                char pidStr[] = "PID";
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
    
    printf("Command: %s\n", userCmd->cmd);

    for (int i = 0; i < argIndex; i++)
    {
        printf("Arg %d: %s length %lu\n", i + 1, userCmd->userArgs[i], strlen(userCmd->userArgs[i]));
    }
    printf("Input redirect: %s\n", userCmd->inputFile);
    printf("Output redirect: %s\n", userCmd->outputFile);
    printf("NumArgs: %d\n", userCmd->numArgs);
    printf("Background (0 for no, 1 for yes): %d\n", userCmd->background);

    return userCmd;

}


int main(void)
{
    int continueProgram = 1;

    while (continueProgram < 2)
    {
    
    struct cmd_elements *currentCmd = getUserInput();
    
    // Debug printing
    printf("IN MAIN --- Command: %s\n", currentCmd->cmd);
    int argIndex = 0;
    while (currentCmd->userArgs[argIndex] != NULL)
    {
        printf("Arg %d: %s\n", argIndex + 1, currentCmd->userArgs[argIndex]);
        argIndex++;
    }
    printf("Input redirect: %s\n", currentCmd->inputFile);
    printf("Output redirect: %s\n", currentCmd->outputFile);
    printf("NumArgs: %d\n", currentCmd->numArgs);
    printf("Background (0 for no, 1 for yes): %d\n", currentCmd->background);

    continueProgram++;
    }

    return 0;
}