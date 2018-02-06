
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
#define MAX_ARGUMENTS MAX_LINE_LENGTH/2  /* and number of arguments */
#define DELIMITER " "                    /* Used to tokenize lines */
#define MAX_COMMANDS 10                  /* Used to determine how far the user can look back at commands */

typedef int bool;
#define FALSE 0
#define TRUE 1


typedef struct history {
    char *commandHistory[MAX_COMMANDS][MAX_LINE_LENGTH + 1];
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


int main(void) {
    char inputBuffer[MAX_LINE_LENGTH + 1];   /* buffer to hold the command entered */
    char *args[MAX_ARGUMENTS + 1];           /* array of arguments */
    pid_t pid;                               /* pid for processes */
    int wstatus;                             /* status code to be used for wait */
    int commandIndex = 0;                    /* Keeps track of what command you are on */
    history ish_history;



    while (TRUE) {
        /* Provide a prompt for the command line */
        printf("%3d ISH> ", commandIndex);
        fflush(stdout);   /* force output to appear */
        commandIndex++;

        /* Get a command line */
        readAndParseArgs(inputBuffer, args);


        /* Checks if there was no input. Prevents a segfault */
        if (num_words_in_sent(inputBuffer) > 0) {

            /* Show the individual arguments */
//            displayArgs(args);

            /* checks if command was internal */
            if (byeCommand(args[0]) == TRUE) {
                return 0;
            } else if (historyCommand(args[0]) == TRUE) {
                logArgument(&ish_history, commandIndex, args);
                showHistory(&ish_history);
                printf("place history function here\n");
            } else if (exclamationCommand(args[0]) == TRUE) {
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
        int testI = 0;
        while (ish_history.commandHistory[0][testI]) {
            testI++;
            printf("%s ", ish_history.commandHistory[0][testI]);
        }
        printf("\n");
    }
}


void showHistory(history *hist) {
    int index = hist->commandNumber;
    int tempComNum = hist->commandNumber;
    int histIn;
    if (index > MAX_COMMANDS) {
        tempComNum = tempComNum - MAX_COMMANDS;
        index++;

        /* Loops through and prints out all reachable commands */
        for (int i = 0; i < MAX_COMMANDS; i++) {
            histIn = 0;
            index = index % MAX_COMMANDS;
            printf("%3d ", tempComNum);
            while (hist->commandHistory[index][histIn] != '\0') {
                printf("%s ", (char *) hist->commandHistory[index][histIn]);
                histIn++;
            }
            tempComNum++;
            printf("\n");
        }
    } else {
        for (int i = 0; i < index; i++) {
            histIn = 0;
            printf("%3d ", i+1);
            while (hist->commandHistory[i][histIn] != NULL) {
                histIn++;
                printf("%s ", hist->commandHistory[i][histIn]);
            }
            printf("\n");
        }
    }
}


bool exclamationCommand(const char *args) {
    return *args == '!' ? TRUE : FALSE;
}



void logArgument(history *hist, int commandNumber, char *args[]) {
    int index;
    for (index = 0; args[index] != NULL; index++) {
        hist->commandHistory[commandNumber % MAX_COMMANDS][index] = args[index];
        printf("%s\n", hist->commandHistory[commandNumber % MAX_COMMANDS][index]);
    }
    hist->commandNumber = commandNumber;
    hist->commandHistory[commandNumber % MAX_COMMANDS][index] = NULL;

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
