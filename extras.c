// This is just some extra functions I used while debugging
// that don't need to be in the final submission

/* displayBgTracker -----------------------------------------------------------
   Description: Prints the contents of the background process tracking array
   TODO: Remove before final submission, this was just used for debugging.
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


// Debug printing
// TODO - remove this before final submission
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