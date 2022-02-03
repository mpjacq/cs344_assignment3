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
#include <errno.h>

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
    char *cmd;
    char *userArgs[512];    // Array of pointers to strings
    int numArgs;
    char *inputFile;
    char *outputFile;
    int background;         // 0 = no, 1 = yes

};


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
struct cmd_elements *getUserInput()
{
    // char userInput[] = "exit";
    char userInput[MAX_CMD_LENGTH * sizeof(char)];
    char *u = userInput;
    size_t buflen = MAX_CMD_LENGTH;
    ssize_t charsRead;

    // Use the colon : symbol as a prompt for each command line. 
    // Flush out the output buffers each time you print
    printf(": ");
    fflush(stdout);

    // The general syntax of a command line is:
    // command [arg1 arg2 ...] [< input_file] [> output_file] [&]
    charsRead = getline(&u, &buflen, stdin);

    // Check if getline was successful, it will return -1 if not
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
    }
    else 
    {
        // getline leaves newline in input, remove this
        userInput[strlen(userInput)-1] = '\0';
    }
    
    // // Check if the first character is a #, this means line is a comment
    // if (userInput[0] == '#')
    // {
    //     // TODO - decide how to properly handle this error
    //     printf("Blank entry\n");
    //     fflush(stdout);
    //     exit(0);
    // }

    // Create a command struct to store data about the command entered
    struct cmd_elements *userCmd = malloc(sizeof(struct cmd_elements));
    
    // Initialize all userArgs elements to NULL
    for (int i = 0; i < 512; i++)
    {
        userCmd->userArgs[i] = (char *) NULL;
    }

     // Extract the command elements
    char *saveptr;
    char *token = strtok_r(userInput, " \n", &saveptr);
    
    if (token != NULL)
    {
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
    }
    // If first token was null, that means no command was entered. Treat is as a comment.
    else
    {
        userCmd->cmd = calloc(strlen("#"), sizeof(char));
        strncpy(userCmd->cmd, "#", (strlen("#") + 1));
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


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void runCommand(struct cmd_elements *currentCmd, int *status){
	
    // Below is adapted from Canvas: https://canvas.oregonstate.edu/courses/1884946/pages/exploration-process-api-monitoring-child-processes?module_item_id=21835973
    // Accessed 1.29.22
    //TODO - need to populate an array with NULL, arg1, arg2...argn...NULL
    // char *newargv[currentCmd->numArgs + 2];
	// int childStatus;
    char *argsWithFrontBuffer[currentCmd->numArgs + 1];

    for (int i = 0; i < (currentCmd->numArgs + 2); i++)
    {
        if (i == 0)
        {
            argsWithFrontBuffer[0] = currentCmd->cmd;
        }
        else if (i == currentCmd->numArgs + 1)
        {
            argsWithFrontBuffer[i] = NULL;
        }
        else
        {
            argsWithFrontBuffer[i] = currentCmd->userArgs[i - 1];
        }
    }

	// Fork a new process
	pid_t spawnPid = fork();

	switch(spawnPid){
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	
    case 0:
		// Child process
		printf("CHILD(%d) attempting to run %s command\n", getpid(), currentCmd->cmd);
        fflush(stdout);
        // sleep(10);

        // Check for input/output redirection (assignment instructions recommended
        // handling this in the CHILD process)
        // Note that after using dup2() to set up the redirection, the redirection 
        // symbol and redirection destination/source are NOT passed into the exec command
        // For example, if the command given is ls > junk, then you handle the 
        // redirection to "junk" with dup2() and then simply pass ls into exec().

        if (currentCmd->inputFile != NULL)
        {
            // An input file redirected via stdin should be opened for reading only; 
            // if your shell cannot open the file for reading, it should print an error 
            // message and set the exit status to 1 (but don't exit the shell).
            int inFile = open(currentCmd->inputFile, O_RDONLY);
            // Check if error
            if (inFile == -1)
            {
                printf("Cannot open %s for input.\n", currentCmd->inputFile);
                fflush(stdout);
                exit(1);
            }
            // If no error, use dup2() to set up redirection
            // inFile will contain the file descriptor
            else
            {
                int dupInResult = dup2(inFile, 0);
                // Check for error - returns -1 on error
                if (dupInResult == -1)
                {
                    printf("Cannot redirect %s for input.\n", currentCmd->inputFile);
                    fflush(stdout);
                    exit(1);
                }
            }

        }

        if (currentCmd->outputFile != NULL)
        {
            // Similarly, an output file redirected via stdout should be opened 
            // for writing only; it should be truncated if it already exists or 
            // created if it does not exist. If your shell cannot open the output 
            // file it should print an error message and set the exit status to 1 
            // (but don't exit the shell).
            int outFile = open(currentCmd->outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            // Check if error
            if (outFile == -1)
            {
                printf("Cannot open %s for output.\n", currentCmd->outputFile);
                fflush(stdout);
                exit(1);
            }
            // If no error, use dup2() to set up redirection
            // outFile will contain the file descriptor
            else
            {
                int dupOutResult = dup2(outFile, 1);
                // Check for error - returns -1 on error
                if (dupOutResult == -1)
                {
                    printf("Cannot redirect %s for output.\n", currentCmd->outputFile);
                    fflush(stdout);
                    exit(1);
                }
            }

        }
		
        execvp(argsWithFrontBuffer[0], argsWithFrontBuffer);

		// exec only returns if there is an error - code below won't be reached unless
        // there was a problem.
        // If a command fails because the shell could not find the command to run, 
        // then the shell will print an error message and set the exit status to 1
		perror("execvp");
        //*status = 1;
        exit(1);
		break;
	
    default:
		// In the parent process
		// Wait for child's termination
		spawnPid = waitpid(spawnPid, status, 0);
			
        printf("Child %d exited normally with status %d\n", spawnPid, WEXITSTATUS(*status));
        //*status = WEXITSTATUS(childStatus);

        // if (WEXITSTATUS(childStatus) == 0)
        // {
        //     printf("Child %d exited normally with status %d\n", spawnPid, WEXITSTATUS(childStatus));
        //     *status = 0;
        // }
        // else
        // {
        //     printf("Child %d exited normally with status %d\n", spawnPid, WEXITSTATUS(childStatus));
        //     *status = WEXITSTATUS(childStatus);
        // }
        fflush(stdout);

        if (WIFSIGNALED(*status))
        {
			printf("Child %d exited abnormally due to signal %d\n", spawnPid, WTERMSIG(*status));
            fflush(stdout);
		}
		break;
	} 
    return;
}

// Debug printing
void printCommand(struct cmd_elements *currentCmd)
{
    printf("---Command: %s | %d Args: ", currentCmd->cmd, currentCmd->numArgs);
    fflush(stdout);
    int argIndex = 0;
    while (currentCmd->userArgs[argIndex] != NULL)
    {
        printf("%s, ", currentCmd->userArgs[argIndex]);
        fflush(stdout);
        argIndex++;
    }
    printf("\n---Input: %s | Output: %s\n", currentCmd->inputFile, currentCmd->outputFile);
    fflush(stdout);
    printf("---Background (0 for no, 1 for yes): %d\n", currentCmd->background);
    fflush(stdout);
}


int main(void)
{
    int status = 0;
    int continueProgram = 1;

    while (continueProgram)
    {
    
        struct cmd_elements *currentCmd = getUserInput();
        
        // Debug printing
        printCommand(currentCmd);

        if (strcmp(currentCmd->cmd, "#") != 0)
        {
            if (strcmp(currentCmd->cmd, "exit") == 0)
            {
                exitCmd();
            }

            if (strcmp(currentCmd->cmd, "status") == 0)
            {
                // Work this into its own function? One for exit status
                // and one for signal?
                if (WIFEXITED(status))
                {
                    printf("Exit value %d\n", WEXITSTATUS(status));
                }
            }

            else if (strcmp(currentCmd->cmd, "cd") == 0)
            {
                char buffer[100];
                int bufsize = 100;
                char buffer2[100];
                int bufsize2 = 100;

                getcwd(buffer, bufsize);
                printf("cwd before calling chDirCmd: %s\n", buffer);
                fflush(stdout);
                chDirCmd(currentCmd);
                getcwd(buffer2, bufsize2);
                printf("cwd after calling chDirCmd: %s\n", buffer2);
                fflush(stdout);
                // /Users/mjacq/CS344/cs344_assignment3
            }
            else
            {
                runCommand(currentCmd, &status);
            }

            // TODO - Dispaying every time for debugging, work into own function
            printf("Status holds: %d\n", status);
            fflush(stdout);
        }
    }

    return 0;
}