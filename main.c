/**
 * Ex2
 * Ori Kopel
 * 205533151
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>


#define MAX_CHARS 20
#define MAX_WORD 20
#define DELIMITER " "
void runShell();

char *getInput();

char **getLexer(char *input);

bool sendToExe(char **lexered);

int main(int argc, char **argv) {
    runShell();
    return 0;
}


void runShell() {

    char *input;
    char **lexered;
    bool hasNext = true;
    while (hasNext) {
        input = getInput();
        lexered = getLexer(input);
        hasNext = sendToExe(lexered);
        free(input);
        free(lexered);
    }


}

char *getInput() {
    char *input = malloc(sizeof(char) * MAX_CHARS);
    int maxLen = MAX_CHARS;
    if (!input) {
        perror("BAD MALLOC");
    }
    char c;
    int pos = 0;
    while (true) {
        c = getchar();
        if (c == '\n' || c == EOF) {
            input[pos] = '\0';
            return input;
        } else {
            input[pos] = c;
            pos++;
        }
        if (pos == maxLen) {
            maxLen += MAX_CHARS;
            input = realloc(input, maxLen);
            if (!input) {
                perror("BAD REALLOC");
            }
        }
    }
}


char **getLexer(char *input) {
    char **lexered = malloc(sizeof(char) * MAX_WORD);
    if(!lexered){
        perror("BAD MALLOC IN LEXER");
    }
    int maxWord=MAX_WORD;
    int pos=0;
    char *tmp = strtok(input, DELIMITER);
    while (tmp) {
        lexered[pos]=tmp;
        pos++;
        if(pos==maxWord){
            maxWord+=MAX_WORD;
            lexered= realloc(lexered,maxWord);
            if(!lexered){
                perror("BAD REALLOC IN LEXER");
            }
        }
        tmp = strtok(input, DELIMITER);
    }
    return lexered;
}

bool sendToExe(char **lexered) {

}
