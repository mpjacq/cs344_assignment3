/* Author: Madeline Jacques (jacquema@oregonstate.edu)
   CS 344 W 22 Assignment 3
   Due Date: 2/7/22
   Last Updated: 2/5/22
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

/* Globals ---------------------------------- */
pid_t bgPIDs[200] = {0};    // Ed post advised 200 background processes max
int bgPIDsSize = 0;
int fgToggle = 0;               // 0 = foreground only mode OFF, 1 = foreground only ON
// Initialize SIGINT_action struct to be empty
struct sigaction SIGINT_action = {0};
struct sigaction SIGTSTP_action = {0};
/* ------------------------------------------- */

/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */

void handleSIGTSTP()
{
    // If foreground mode not on, turn on
    if (fgToggle == 0)
    {
        fgToggle = 1;
        // Display message immediately if at prompt or after current processes terminate
        char *fgOn = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, fgOn, 51);
    }
    else if (fgToggle == 1)
    {
        fgToggle = 0;
        // Display message immediately if at prompt or after current processes terminate
        char *fgOff = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, fgOff, 31);
    }
}

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
void addBgPID(pid_t pid)
{
    // Cycle through the array till you find an empty spot
    int found = 0;
    int index = 0;

    while (found == 0 && index < 200)
    {
        if (bgPIDs[index] == 0)
        {
            bgPIDs[index] = pid;    // Add PID by replacing 0 with PID
            found++;
        }
        index++;
    }
    // If no spot was found, print message
    if (found == 0)
    {
        printf("Background PID array is full, %d not added.\n", pid);
        fflush(stdout);
    }
    else
    {
        bgPIDsSize++;     // Increment size tracker
    }
    return;
}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void removeBgPID(pid_t pid)
{
    int found = 0;
    int index = 0;

    // Cycle through the array till you find the matching PID
    while (found == 0 && index < 200)
    {
        if (bgPIDs[index] == pid)
        {
            bgPIDs[index] = 0;      // Remove PID by replacing with 0
            found++;
        }
        index++;
    }
    // If no match was found, print message
    if (found == 0)
    {
        printf("No background PID found for %d, not removed.\n", pid);
        fflush(stdout);
    }
    else
    {
        bgPIDsSize--;       // Decrement size tracker
    }
    return;
}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void displayBgTracker()
{
    for (int i = 0; i < 200; i++)
    {
        if (bgPIDs[i] != 0)
        {
            printf("[%d]: %d ", i, bgPIDs[i]);
            fflush(stdout);
        }
    }
    printf("\nDone printing, size of array is %d/200.\n", bgPIDsSize);
    fflush(stdout);
    return;
}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void checkBackground()
{
    // Your parent shell will need to periodically check for the background child 
    // processes to complete, so that they can be cleaned up, as the shell continues 
    // to run and process commands. The time to print out when these background 
    // processes have completed is just BEFORE command line access and control 
    // are returned to the user, every time that happens.
    pid_t bkgPID;
    pid_t waitResult;
    int bkgStatus;
    int index = 0;
    
    if (bgPIDsSize > 0)
    {
        // use wait to check if there is a PID waiting to be claimed 
        // in the background. if it is 0, there is nothing waiting.
        // otherwise use the PID returned and remove from tracker.        
        while (index < 200)
        {
            if (bgPIDs[index] != 0)
            {
                bkgPID = bgPIDs[index];
                // printf("Found a background process %d to check for\n", bkgPID);
                // fflush(stdout);

                waitResult = waitpid(bkgPID, &bkgStatus, WNOHANG);

                if (waitResult != 0)
                {
                    // printf("Background process %d exited, status %d\n", bkgPID, bkgStatus);
                    // fflush(stdout);
                    // removeBgPID(bkgPID);

                    if (WIFEXITED(bkgStatus))
                    {
                        printf("Background PID %d is done: exit value %d\n", bkgPID, WEXITSTATUS(bkgStatus));
                        fflush(stdout);
                        removeBgPID(bkgPID);
                    } 
                    else if (WIFSIGNALED(bkgStatus))
                    {
                        printf("Background PID %d is done: terminated by signal %d\n", bkgPID, WTERMSIG(bkgStatus));
                        fflush(stdout);
                        removeBgPID(bkgPID);
                    }
                }
                // else
                // {
                //     printf("Background process %d is not done yet...\n", bkgPID);
                //     fflush(stdout);
                // }
            }
            index++;
        }           
    }
}


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
    ssize_t charsRead = 0;

    while (charsRead <= 0)
    {
        // Use the colon : symbol as a prompt for each command line. 
        // Flush out the output buffers each time you print
        printf(": ");
        fflush(stdout);

        // The general syntax of a command line is:
        // command [arg1 arg2 ...] [< input_file] [> output_file] [&]
        
        charsRead = getline(&u, &buflen, stdin);

        // Check if getline was successful, it will return -1 if not
        // Will also return -1 if interrupted by Ctrl-Z (SIGTSTP), so
        // need to clear error and re-prompt for command
        if (charsRead == -1)
        {
            clearerr(stdin);
        }
    }

    // getline leaves newline in input, remove this
    if (charsRead > 0)
    {
        userInput[strlen(userInput)-1] = '\0';
    }

    // Create a command struct to store data about the command entered
    struct cmd_elements *userCmd = malloc(sizeof(struct cmd_elements));
    
    // Initialize all userArgs elements to NULL
    for (int i = 0; i < 512; i++)
    {
        userCmd->userArgs[i] = NULL;
    }
    
    // Initialize in/out files to NULL
    userCmd->inputFile = NULL;
    userCmd->outputFile = NULL;
    userCmd->numArgs = 0;
    userCmd->background = 0;

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
    if (userCmd->numArgs > 0)
    {
        if (userCmd->userArgs[userCmd->numArgs - 1][0] == '&' && strlen(userCmd->userArgs[userCmd->numArgs - 1]) == 1)
        {
            userCmd->background = 1;
            // This also cuts off any args entered after & which should not be
            // considered valid anyways.
            userCmd->userArgs[userCmd->numArgs - 1] = NULL;
            userCmd->numArgs--;
        }
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
    // Free dynamically allocated mem of command?
    // Check for incomplete background processes
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
}


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void inputRedirect(struct cmd_elements *currentCmd)
{
    int inFile = 0;
    // An input file redirected via stdin should be opened for reading only; 
    // if your shell cannot open the file for reading, it should print an error 
    // message and set the exit status to 1 (but don't exit the shell).
    if (currentCmd->background == 0)
    {
        inFile = open(currentCmd->inputFile, O_RDONLY);
    }
    else
    {
        inFile = open("/dev/null", O_RDONLY);
    }
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


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void outputRedirect(struct cmd_elements *currentCmd)
{
    // Similarly, an output file redirected via stdout should be opened 
    // for writing only; it should be truncated if it already exists or 
    // created if it does not exist. If your shell cannot open the output 
    // file it should print an error message and set the exit status to 1 
    // (but don't exit the shell).
    int outFile = 0;

    if (currentCmd->background == 0)
    {
       outFile = open(currentCmd->outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    }
    else
    {
        outFile = open("/dev/null", O_CREAT | O_WRONLY | O_TRUNC, 0777);
    }
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


/* functionName -----------------------------------------------------------
   Description: Purpose and general use of the function t/o the application
   Arguments: Identify and describe all arguments
              *Include any assumptions related to the data
   Returns: Identify and describe return data
----------------------------------------------------------------------- */
void runCommand(struct cmd_elements *currentCmd, int *status, int *statusType){
	
    // Below is adapted from Canvas: https://canvas.oregonstate.edu/courses/1884946/pages/exploration-process-api-monitoring-child-processes?module_item_id=21835973
    // Accessed 1.29.22
	int childStatus;
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

	switch(spawnPid)
    {
        case -1:
            perror("fork()\n");
            exit(1);
            break;
	
        case 0:
            // Child process
            // printf("CHILD(%d) attempting to run %s command\n", getpid(), currentCmd->cmd);
            // fflush(stdout);
            // sleep(10);

            // SIGNAL HANDLING ----------------------------
            // Child FOREGROUND process should no longer ignore SIGINT (Ctrl-C) 
            // and should TERMINATE ITSELF upon receipt of this signal.
            // (Background children should still ignore Ctrl-C)
            if (currentCmd->background == 0)
            {
                SIGINT_action.sa_handler = SIG_DFL;         // Restore default behavior
	            sigaction(SIGINT, &SIGINT_action, NULL);    // Install
            }
            // Both foreground and and background children should ignore SIGTSTP (Ctrl-Z)
            SIGTSTP_action.sa_handler = SIG_IGN;
            sigaction(SIGTSTP, &SIGTSTP_action, NULL);

            // Check for input/output redirection (assignment instructions recommended
            // handling this in the CHILD process)
            // Note that after using dup2() to set up the redirection, the redirection 
            // symbol and redirection destination/source are NOT passed into the exec command
            // For example, if the command given is ls > junk, then you handle the 
            // redirection to "junk" with dup2() and then simply pass ls into exec().

            // If the user doesn't redirect the standard input for a background command, 
            // then standard input should be redirected to /dev/null
            if ((currentCmd->background == 0 && currentCmd->inputFile != NULL) || 
                (currentCmd->background == 1 && currentCmd->inputFile == NULL))
            {
                inputRedirect(currentCmd);
            }
            // If the user doesn't redirect the standard output for a background command, 
            // then standard output should be redirected to /dev/null
            if ((currentCmd->background == 0 && currentCmd->outputFile != NULL) ||
                (currentCmd->background == 1 && currentCmd->outputFile == NULL))
            {
                outputRedirect(currentCmd);
            }
            
            execvp(argsWithFrontBuffer[0], argsWithFrontBuffer);

            // exec only returns if there is an error - code below won't be reached unless
            // there was a problem.
            // If a command fails because the shell could not find the command to run, 
            // then the shell will print an error message and set the exit status to 1
            perror(currentCmd->cmd);
            fflush(stdout);
            *status = 1;
            exit(1);
	
        default:
            
            // Are we supposed to run this in the background?
            if (currentCmd->background == 1 && fgToggle == 0)
            {
                // Keep this - required output - see example
                printf("Background PID is %d, command %s\n", spawnPid, currentCmd->cmd);
                fflush(stdout);
                
                // The act of calling wait to reap the exit status is itself what
                // recovers a zombie process.
                // If the background process was terminated super fast, we can go ahead and reap
                // Otherwise, using WNOHANG lets the program continue on (will return 0)
                pid_t backgoundChild = waitpid(spawnPid, &childStatus, WNOHANG);

                // If we didn't collect the child PID because it wasn't finished,
                // we need to add its PID to the tracker so we can check on it later
                if (backgoundChild == 0)
                {
                    addBgPID(spawnPid);
                }
            }
            // If foreground, wait for child's termination
            else
            {
                spawnPid = waitpid(spawnPid, &childStatus, 0);
                // printf("waitpid returned value %d\n", spawnPid);
                // fflush(stdout);
                    
                if (WIFEXITED(childStatus))
                {
                    // printf("Child %d exited normally with status %d\n", spawnPid, WEXITSTATUS(childStatus));
                    // fflush(stdout);
                    *status = WEXITSTATUS(childStatus);
                    *statusType = 0;
                } 
                // If a child foreground process is killed by a signal, the parent must immediately 
                // print out the number of the signal that killed it's foreground child process 
                // before prompting the user for the next command.
                else if (WIFSIGNALED(childStatus))
                {
                    printf("Terminated by signal %d\n", WTERMSIG(childStatus));
                    *status = WTERMSIG(childStatus);
                    *statusType = 1;
                    fflush(stdout);
                }
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
    int statusType = 0;         // 0 = exit code, 1 = signal
    int continueProgram = 1;

    // SIGNAL HANDLING ----------------------------------------------
	// SIGINT handling: parent shell should ignore SIGINT
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	// No flags set
	SIGINT_action.sa_flags = 0;
	// Install our signal handler
	sigaction(SIGINT, &SIGINT_action, NULL);

    // SIGTSTP signal handling
    SIGTSTP_action.sa_handler = handleSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // --------------------------------------------------------------

    while (continueProgram)
    {
        // Get new command
        struct cmd_elements *currentCmd = getUserInput();
        
        // Debug printing
        // printCommand(currentCmd);
        // fflush(stdout);

        if (currentCmd->cmd[0] != '#')
        {
            if (strcmp(currentCmd->cmd, "exit") == 0)
            {
                // Ignore background setting for built-ins
                currentCmd->background = 0;
                // Clean up memory before calling exit - either put in
                // exit fc or make cleanup its own fc.
                exitCmd();
            }

            if (strcmp(currentCmd->cmd, "status") == 0)
            {
                // Ignore background setting for built-ins
                currentCmd->background = 0;
                // Work this into its own function? One for exit status
                // and one for signal?
                // Status only tracks foreground processes! 
                if (statusType == 0)
                {
                    printf("Exit value %d\n", status);
                    fflush(stdout);
                }
                else if (statusType == 1)
                {
                    printf("Terminated by signal %d\n", status);
                    fflush(stdout);
                }
            }

            else if (strcmp(currentCmd->cmd, "cd") == 0)
            {
                char buffer[100];
                int bufsize = 100;
                char buffer2[100];
                int bufsize2 = 100;

                // Ignore background setting for built-ins
                currentCmd->background = 0;

                getcwd(buffer, bufsize);
                chDirCmd(currentCmd);
                getcwd(buffer2, bufsize2);
            }
            else
            {
                runCommand(currentCmd, &status, &statusType);
            }
        }
        // The time to print out when these background processes have completed 
        // is just BEFORE command line access and control are returned to the user, 
        // every time that happens.
        checkBackground();

        // // Free memory - parts of command struct are dynamically allocated
        if (currentCmd->cmd != NULL)
        {
            free(currentCmd->cmd);
        }
        if (currentCmd->numArgs > 0)
        {
            // currentCmd->userArgs array itself was not dynamically allocated,
            // but its contents were
            for (int i = 0; i < currentCmd->numArgs; i++)
            {
                if (currentCmd->userArgs[i] != NULL)
                {
                    free(currentCmd->userArgs[i]);
                }
            }
        }
        if (currentCmd->inputFile)
        {
            free(currentCmd->inputFile);
        }
        if (currentCmd->outputFile)
        {
            free(currentCmd->outputFile);
        }
        free(currentCmd);
    }

    return 0;
}