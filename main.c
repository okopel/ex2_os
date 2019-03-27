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
#define MAX_JOBS 50
struct Task {
    char *name;
    int pid;
};

struct Task jobs[MAX_JOBS];


void runShell();

char *getInput();

char **getLexer(char *input);

bool sendToExe(int index, char **lexered);

bool waitingCheck(char **lexered);

void waitToChild(int pid);

void dontWait();

void parent(int index, bool wait, char *name, int pid);

void child(char **lexered);

void showJobs();

void addJob(int jobPos, char *name, int pid);

void removeJob(char *name, int pid);

int orderJobs();

void cdFunc();

int main(int argc, char **argv) {
//    jobPos = 0;
    runShell();
    return 0;
}

void runShell() {
    char *input;
    char **lexered;
    bool hasNext = true;
    int jobPos = 0;
    while (hasNext) {
        jobPos++;
        input = getInput();
        lexered = getLexer(input);
        hasNext = sendToExe(jobPos - 1, lexered);
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

bool sendToExe(int index, char **lexered) {
    bool isWait = waitingCheck(lexered);
    pid_t pid;
    pid = fork();
    if (pid == -1) {//Error case
        perror("ERROR IN FORK\n");
    } else if (pid > 0) {
        parent(index, isWait, strdup(lexered[0]), pid);
    } else if (pid == 0) {
        child(lexered);
        exit(1);
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

    do {
        wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void parent(int index, bool wait, char *name, int pid) {
    addJob(index, strdup(name), pid);
    printf("PID=%d\n", pid);
    if (!wait) {
        dontWait();
    } else {
        waitToChild(pid);
    }
    removeJob(strdup(name), pid);
}

void child(char **lexered) {
    if (strcmp(lexered[0], "jobs") == 0) {
        showJobs();
    } else if (strcmp(lexered[0], "cd") == 0) {
        cdFunc();
    } else if (execvp(lexered[0], lexered) == -1) {
        perror("ERROR IN EXE\n");
    }
}

void showJobs() {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        if ((jobs[i].pid != 0) && (jobs[i].name != NULL)) {
            printf("%d)%d %s\n", i, jobs[i].pid, jobs[i].name);
        }
    }
}

void addJob(int jobPos, char *name, int pid) {
    if (jobPos < MAX_JOBS) {
        jobs[jobPos].name = strdup(name);
        jobs[jobPos].pid = pid;
    } else {
        int newJobPos = orderJobs();
        addJob(newJobPos, strdup(name), pid);
    }
}

void removeJob(char *name, int pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        if (jobs[i].pid == pid) {
            jobs[i].name = "";
            jobs[i].pid = 0;
            return;
        }
    }
    printf("There isnt job to remove\n");
}

int orderJobs() {
    printf("!");
    //return 5;
    //  printf("orderJobs\n");
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        if ((strcmp(jobs[i].name, "") == 0) && (strcmp(jobs[i + 1].name, "") != 0)) {
            strcpy(jobs[i].name, jobs[i + 1].name);
            strcpy(jobs[i + 1].name, "");

            jobs[i].pid = jobs[i + 1].pid;
            jobs[i + 1].pid = 0;
        }
    }
    printf("!");
    int jobIndex = MAX_JOBS;
    while (jobIndex > 0) {
        printf("!");
        if (jobs[jobIndex].name != NULL) {
            return jobIndex + 1;
        }
        jobIndex--;
    }
    //printf("EndOfOrder\n");
}

void cdFunc() {
    printf("CD\n");//todo
}