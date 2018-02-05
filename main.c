
/*
  ISH: Incredibly Simple Shell

  Author:
  MAT 4970
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_LINE_LENGTH 80               /* upper bounds on line */
#define MAX_ARGUMENTS MAX_LINE_LENGTH/2  /* and number of arguments */
#define DELIMITER " "                    /* Used to tokenize lines */


typedef int bool;
#define FALSE 0
#define TRUE 1

void readAndParseArgs(char inputBuffer[], char *args[]);
void displayArgs(char *args[]);
int num_words_in_sent(char *sentence);




int main(void) {
    char inputBuffer[MAX_LINE_LENGTH + 1];   /* buffer to hold the command entered */
    char *args[MAX_ARGUMENTS + 1];           /* array of arguments */
    pid_t pid;

    while (TRUE) {
        /* Provide a prompt for the command line */
        printf("ISH> ");
        fflush(stdout);   /* force output to appear */

        /* Get a command line */
        readAndParseArgs(inputBuffer, args);

        /* Show the individual arguments */
        displayArgs(args);

        /* Create a child process, execute the specified command,
           and wait for completion */
        
    }
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

    printf("Line entered = \"%s\"\n", inputBuffer);



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
