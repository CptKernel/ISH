
/*
  ISH: Incredibly Simple Shell

  Author: Ethan Cunningham
  MAT 4970
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define MAX_LINE_LENGTH 80               /* upper bounds on line */
#define MAX_ARGUMENTS (MAX_LINE_LENGTH/2)  /* and number of arguments */
#define DELIMITER " "                    /* Used to tokenize lines */
#define MAX_COMMANDS 10                  /* Used to determine how far the user can look back at commands */

typedef int bool;
#define FALSE 0
#define TRUE 1


typedef struct history {
    char commandHistory[MAX_COMMANDS][MAX_LINE_LENGTH + 1];
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
void parseArgs(history *hist, char *args[], int commandToGet);
void executeHistoryCommand(char *args[], history *hist);

int main(void) {
    char inputBuffer[MAX_LINE_LENGTH + 1];   /* buffer to hold the command entered */
    char *args[MAX_ARGUMENTS + 1];           /* array of arguments */
    pid_t pid;                               /* pid for processes */
    int wstatus;                             /* status code to be used for wait */
    int commandIndex = 0;                    /* Keeps track of what command you are on */
    history ish_history;
    int zomCount = 0;



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

        /* If it is an &, No longer want it being read as an instruction here */
        if (strcmp(lastArg, "&") == 0) {
            args[pIndex - 1] = NULL;
        }

        /* Checks if there was no input. Prevents a segfault */
        if (num_words_in_sent(inputBuffer) > 0) {

            /* checks if command was internal */
            if (byeCommand(args[0]) == TRUE) {
                return 0;
            } else if (strcmp(lastArg, "&") == 0) {

                if ((pid = fork()) < 0) {
                    fprintf(stderr, "Process could not fork.\n");
                } else if (pid == 0) {
                    if (execvp(*args, args) < 0) {
                        fprintf(stderr, "ERROR in execvp call\n Make sure the input commands were correct\n");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    printf("[%d] %d\n", ++zomCount, pid);
                    fflush(stdout);
                }
            } else if (historyCommand(args[0]) == TRUE) {
                showHistory(&ish_history);
                logArgument(&ish_history, commandIndex, args);
            } else if (exclamationCommand(args[0]) == TRUE) {
//                executeHistoryCommand(args, ish_history);
                logArgument(&ish_history, commandIndex, args);
                printf("Insert command history exclamation function here.\n");
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


void parseArgs(history *hist, char *args[], int commandToGet) {
    char *word;
    int currentLocation = hist->commandNumber;
    if (currentLocation - commandToGet > MAX_COMMANDS) {
        fprintf(stderr, "Cannot look that far back in command history.\n");
        return;
    }


}






void showHistory(history *hist) {
    printf("\n");
    int tempCommandNumber = hist->commandNumber;
    int displayCommandNumber;
    if (tempCommandNumber > 10) {
        displayCommandNumber = hist->commandNumber - 9;
        tempCommandNumber++;
        while ((tempCommandNumber %  MAX_COMMANDS) != (hist->commandNumber % MAX_COMMANDS)) {
            printf("%3d ", displayCommandNumber++);
            for (int i = 0; hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i] != NULL; i++) {
                printf("%c", hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i]);
            }
            tempCommandNumber++;
            printf("\n");
        }
        printf("%3d ", displayCommandNumber);
        for (int i = 0; hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i] != NULL; i++) {
            printf("%c", hist->commandHistory[tempCommandNumber % MAX_COMMANDS][i]);
        }
        printf("\n");
    } else {
        displayCommandNumber = 0;
        while (displayCommandNumber <= hist->commandNumber) {
            printf("%3d ", displayCommandNumber);
            for (int i = 0; hist->commandHistory[displayCommandNumber][i] != NULL; i++) {
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
        for (histIndex; temp[charIndex] != NULL; histIndex++) {
            hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex] = temp[charIndex++];
        }
        hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex++] = ' ';
    }
    hist->commandNumber = commandNumber;
    hist->commandHistory[commandNumber % MAX_COMMANDS][histIndex - 1] = NULL;

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
    int numWords = num_words_in_sent(inputBuffer);
    if (numWords == 0) {
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
