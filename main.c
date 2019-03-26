/**
 * Ex2
 * Ori Kopel
 * 205533151
 */



#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHARS 256
#define MAX_WORD 8
#define DELIMITER " "
#define WAITING_DELIMITER '&'


struct Task {
    char *name;
    int pid;
    struct Task *next;
};


void runShell();

char *getInput();

char **getLexer(char *input);

bool sendToExe(char **lexered);

bool waitingCheck(char **lexered);

void waitToChild(int pid);

void dontWait();

void parent(bool wait, int pid);

void child(char **lexered);

struct Task *head = NULL;
struct Task *p = NULL;

void showJobs();

void addJob(struct Task *t);

void removeJob(struct Task *newT);

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
    if (!lexered) {
        perror("BAD MALLOC IN LEXER");
    }
    int maxWord = MAX_WORD;
    int pos = 0;
    char *tmp = strtok(input, DELIMITER);

    while (tmp) {
        lexered[pos] = tmp;
        pos++;
        if (pos == maxWord) {
            maxWord += MAX_WORD;
            lexered = realloc(lexered, maxWord);
            if (!lexered) {
                perror("BAD REALLOC IN LEXER");
            }
        }
        tmp = strtok(NULL, DELIMITER);
    }
    lexered[pos] = NULL;

    return lexered;
}

bool sendToExe(char **lexered) {
    bool isWait = waitingCheck(lexered);
    pid_t pid;
    pid = fork();
    if (pid == -1) {//Error case
        perror("ERROR IN FORK");
    } else if (pid == 0) {
        printf("child\n");
        child(lexered);
        printf("endOfChild\n");
    } else if (pid > 0) {
        parent(isWait, pid);

    }
    return true;
}

bool waitingCheck(char **lexered) {
    if (lexered == NULL || lexered[0] == NULL) {
        return false;
    }
    int i = 0;
    while (lexered[i] != NULL) {
        i++;
    }
    if (lexered[i - 1][strlen(lexered[i - 1]) - 1] == WAITING_DELIMITER) {
        lexered[i - 1][strlen(lexered[i - 1]) - 1] = '\0';
        return true;
    }

    return false;
}

void dontWait() {
    printf("D-WAIT\n");
}

void waitToChild(int pid) {
    printf("WAIT\n");
    pid_t wpid;
    int status;
    do {//todo
        wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void parent(bool wait, int pid) {
    printf("PID=%d\n", pid);
    if (!wait) {
        dontWait();
    } else {
        waitToChild(pid);
    }
}

void child(char **lexered) {
    printf("ok");
    struct Task *t;
    t->name = lexered[0];
    t->pid = getpid();
    printf("ok2");
    addJob(t);
    printf("ok3");
    if (lexered[0] == "jobs") {
        printf("ok4");
        removeJob(t);
        printf("ok5");
        showJobs();
        printf("ok6");
        exit(4);

    } else if (lexered[0] == "cd") {

    } else if (execvp(lexered[0], lexered) == -1) {
        perror("ERROR IN EXE");
    }
    printf("ok7");
    removeJob(t);
    printf("ok8");
    exit(3);
}

void showJobs() {
    struct Task *t = head;
    if (t == NULL) {
        printf("there isnt jobs");

    }
    while (t != NULL) {
        printf("%d %s\n", t->pid, t->name);
        t = t->next;
    }
}

void addJob(struct Task *newT) {
    if (head == NULL) {
        struct Task *t;
        t->name = "";
        t->pid = 0;
        t->next = p;
        head = t;
    }
    p->next = newT;
    p = p->next;
}

void removeJob(struct Task *newT) {
    struct Task *t = head;
    struct Task *dad = NULL;
    while (t != NULL && newT != t) {
        dad = t;
        t = t->next;
    }
    dad->next = t->next;
    free(t);
}