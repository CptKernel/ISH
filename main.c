
/*
  ISH: Incredibly Simple Shell

  Author: Ethan Cunningham
  MAT 4970
*/

#include <stdio.h>                                                                                                                                                                    /* Ethan Cunningham */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define MAX_LINE_LENGTH 80               /* upper bounds on line */
#define MAX_ARGUMENTS (MAX_LINE_LENGTH/2)  /* and number of arguments */
#define DELIMITER " "                    /* Used to tokenize lines */
#define MAX_COMMANDS_START  10
int MAX_COMMANDS = 10;                  /* Used to determine how far the user can look back at commands */

typedef int bool;
#define FALSE 0
#define TRUE 1


typedef struct history {
    char commandHistory[MAX_COMMANDS_START][MAX_LINE_LENGTH + 1];
    int commandNumber;
} history;


void logArgument(history *hist, int commandNumber, char *args[]);
void readAndParseArgs(char inputBuffer[], char *args[]);
void displayArgs(char *args[]);
int num_words_in_sent(char *sentence);
bool byeCommand(char *args);
bool historyCommand(char *args);
bool exclamationCommand(const char *args);
void showHistory(history *hist);
void executeHistoryCommand(char *args[], history *hist);

int main(void) {
    char inputBuffer[MAX_LINE_LENGTH + 1];   /* buffer to hold the command entered */
    char *args[MAX_ARGUMENTS + 1];           /* array of arguments */
    pid_t pid;                               /* pid for processes */
    int wstatus;                             /* status code to be used for wait */
    int commandIndex = 1;                    /* Keeps track of what command you are on */
    history ish_history;
    char currentWorkingDir[2048];

    while (TRUE) {
        /* Provide a prompt for the command line */
        printf("%-3d ISH> ", commandIndex);
        fflush(stdout);   /* force output to appear */

        /* Get a command line */
        readAndParseArgs(inputBuffer, args);

        /* This is grabbing the last arg to see if it is an & */
        int pIndex;
        char *lastArg = "";
        for (pIndex = 0; args[pIndex] != NULL; pIndex++) {
            lastArg = args[pIndex];
        }



        /* Checks if there was no input. Prevents a segfault */
        if (num_words_in_sent(inputBuffer) > 0) {

            /* checks if command was internal */
            if (byeCommand(args[0]) == TRUE) {
                return 0;
            } else if (strcmp(lastArg, "&") == 0) {
                logArgument(&ish_history, commandIndex, args);

                /* If it is an &, No longer want it being read as an instruction here */
                if (strcmp(lastArg, "&") == 0) {
                    args[pIndex - 1] = NULL;
                }

                if ((pid = fork()) < 0) {
                    fprintf(stderr, "Process could not fork.\n");
                } else if (pid == 0) {
                    if (execvp(*args, args) < 0) {
                        fprintf(stderr, "ERROR in execvp call\n Make sure the input commands were correct\n");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    printf("%d\n", pid);
                    waitpid(pid, &wstatus, WNOHANG);
                    fflush(stdout);
                }
            } else if (historyCommand(args[0]) == TRUE) {
                showHistory(&ish_history);
                logArgument(&ish_history, commandIndex, args);
            } else if (exclamationCommand(args[0]) == TRUE) {
                executeHistoryCommand(args, &ish_history);
                logArgument(&ish_history, commandIndex, args);
            } else if (strcmp(args[0], "cd") ==  0) {
                logArgument(&ish_history, commandIndex, args);
                chdir(args[1]);
            } else if (strcmp(args[0], "pwd") == 0) {
                logArgument(&ish_history, commandIndex, args);
                getcwd(currentWorkingDir, sizeof(currentWorkingDir));
                printf("%s\n", currentWorkingDir);
            } else {

                /* Logging the argument */
                logArgument(&ish_history, commandIndex, args);

                /* Create a child process, execute the specified command,
                   and wait for completion */

                if ((pid = fork()) < 0) {
                    fprintf(stderr, "Process could not fork.\n");
                } else if (pid == 0) {
                    if (execvp(*args, args) < 0) {
                        fprintf(stderr, "ERROR in execvp call\n Make sure the input commands were correct\n");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    if (wait(&wstatus) < 0) {
                        fprintf(stderr, "ERROR occurred while the parent was waiting for child\n");
                    }
                }
            }
        }
        commandIndex++;
    }
}


void executeHistoryCommand(char *args[],  history *hist) {
    pid_t pid;
    char currentWorkingDir[2048];
    int wstatus;
    int historyNumber = 0;
    char *exclamationString = args[0];
    int argsLength = (int) strlen(exclamationString);
    char number[argsLength];
    for (int i = 1; i < argsLength; i++) {
        number[i - 1] = exclamationString[i];
    }
    number[argsLength - 1] = '\0';
    historyNumber = atoi(number);
    historyNumber = historyNumber % MAX_COMMANDS;
    int tempIndex = 0;
    int argsIndex = 0;
    char inputBuffer[MAX_LINE_LENGTH + 1];
    while (hist->commandHistory[historyNumber][tempIndex] != '\0') {
        inputBuffer[tempIndex] = hist->commandHistory[historyNumber][tempIndex];
        tempIndex++;
    }
    inputBuffer[tempIndex] = '\0';

    int numWordsEHC = num_words_in_sent(inputBuffer);
    if (numWordsEHC == 0) {
        return;
    }

    /* Isolate the arguments found on the input line and
       store them in the argument vector */
    char *ptr;
    ptr = strtok(inputBuffer, DELIMITER);
    int index = 0;

    while (ptr != NULL) {
        args[index] = ptr;

        index++;
        ptr = strtok(NULL, DELIMITER);
    }
    args[index] = NULL;

    /* This is grabbing the last arg to see if it is an & */
    int pIndex;
    char *lastArg = "";
    for (pIndex = 0; args[pIndex] != NULL; pIndex++) {
        lastArg = args[pIndex];
    }

    /* If it is an &, No longer want it being read as an instruction here */
    if (strcmp(lastArg, "&") == 0) {
        args[pIndex - 1] = NULL;
    }


    /* Checks if there was no input. Prevents a segfault */
    if (num_words_in_sent(inputBuffer) > 0) {

        /* checks if command was internal */
        if (strcmp(lastArg, "&") == 0) {
            if ((pid = fork()) < 0) {
                fprintf(stderr, "Process could not fork.\n");
            } else if (pid == 0) {
                if (execvp(*args, args) < 0) {
                    fprintf(stderr, "ERROR in execvp call\n Make sure the input commands were correct\n");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            } else {
                printf("%d\n", pid);
                waitpid(pid, &wstatus, WNOHANG);
                fflush(stdout);
            }
        } else if (historyCommand(args[0]) == TRUE) {
            showHistory(hist);
        } else if (strcmp(args[0], "cd") ==  0) {
            chdir(args[1]);
            printf("\n");
        } else if (strcmp(args[0], "pwd") == 0) {
            getcwd(currentWorkingDir, sizeof(currentWorkingDir));
            printf("%s\n", currentWorkingDir);
        } else {

            /* Create a child process, execute the specified command,
               and wait for completion */

            if ((pid = fork()) < 0) {
                fprintf(stderr, "Process could not fork.\n");
            } else if (pid == 0) {
                if (execvp(*args, args) < 0) {
                    fprintf(stderr, "ERROR in execvp call\n Make sure the input commands were correct\n");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            } else {
                if (wait(&wstatus) < 0) {
                    fprintf(stderr, "ERROR occurred while the parent was waiting for child\n");
                }
            }
        }
    }
}



void showHistory(history *hist) {
    printf("\n");
    int tempCommandNumber = hist->commandNumber;
    int displayCommandNumber;
    if (tempCommandNumber >= MAX_COMMANDS) {
        displayCommandNumber = hist->commandNumber - MAX_COMMANDS + 1;
        tempCommandNumber++;
        while ((tempCommandNumber %  MAX_COMMANDS) != (hist->commandNumber % MAX_COMMANDS)) {
            printf("%3d ", displayCommandNumber++);
            for (int i = 0; hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i] != '\0'; i++) {
                printf("%c", hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i]);
            }
            tempCommandNumber++;
            printf("\n");
        }
        printf("%3d ", displayCommandNumber);
        for (int i = 0; hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i] != '\0'; i++) {
            printf("%c", hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i]);
        }
        printf("\n");
    } else {
        displayCommandNumber = 1;
        while (displayCommandNumber <= hist->commandNumber) {
            printf("%3d ", displayCommandNumber);
            for (int i = 0; hist->commandHistory[displayCommandNumber][i] != '\0'; i++) {
                printf("%c", hist->commandHistory[displayCommandNumber][i]);
            }
            displayCommandNumber++;
            printf("\n");
        }
    }
    printf("\n");

}


bool exclamationCommand(const char *args) {
    return *args == '!' ? TRUE : FALSE;
}



void logArgument(history *hist, int commandNumber, char *args[]) {
    int argsIndex;
    int histIndex = 0;
    for (argsIndex = 0; args[argsIndex] != NULL; argsIndex++) {
        char *temp = args[argsIndex];
        int charIndex = 0;
        for (histIndex; temp[charIndex] != '\0'; histIndex++) {
            hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex] = temp[charIndex++];
        }
        hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex++] = ' ';
    }
    hist->commandNumber = commandNumber;
    hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex - 1] = '\0';

}


bool historyCommand(char *args) {
    return strcmp(args, "history") == 0 ? TRUE : FALSE;
}


bool byeCommand(char *args) {
    return strcmp(args, "bye") == 0 ? TRUE : FALSE;
}


/* Used to count the number of words in a line. */
int num_words_in_sent(char *sentence) {
    int sum = 0;
    char *temp;
    if (strlen(sentence) == 0) {
        return 0;
    }
    while(*sentence != '\0') {

        if (*sentence == ' ' || *sentence == '\t') {

            temp = sentence;
            temp++;

            /* Looks to see if the next char in the sentence is a ' ' , '\t', or NULL */
            if (*temp != '\0' && *temp != ' ' && *temp != '\t') {
                sum++;
            }
            sentence++;
        } else {
            sentence++;
        }
    }
    sum++;
    return sum;
}


void readAndParseArgs(char inputBuffer[], char *args[]) {
    int length;   /* number of characters read from the input */

    /* get a line of input from the standard input file */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE_LENGTH + 1);
    if (length == 0) {
        /* EOF encountered... quit the shell */
        printf("exit\n");
        exit(0);
    }

    /* Transform the inputBuffer to a C-style string */
    inputBuffer[length - 1] = '\0';

//    printf("Line entered = \"%s\"\n", inputBuffer);
    int numWordsRAPA = num_words_in_sent(inputBuffer);
    if (numWordsRAPA == 0) {
        return;
    }

    /* Isolate the arguments found on the input line and
       store them in the argument vector */
    char *ptr;
    ptr = strtok(inputBuffer, DELIMITER);
    int index = 0;

    while (ptr != NULL) {
        args[index] = ptr;

        index++;
        ptr = strtok(NULL, DELIMITER);
    }
    args[index] = NULL;
}



void displayArgs(char *args[]) {
    /* Output the arguments in a given argument vector */
    int index = 0;
    while (args[index] != NULL) {
        printf("%s ", args[index]);
        index++;
    }
    printf("\n");
}
