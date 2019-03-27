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
int jobPos;

void runShell();

char *getInput();

char **getLexer(char *input);

bool sendToExe(char **lexered);

bool waitingCheck(char **lexered);

void waitToChild(int pid);

void dontWait();

void parent(bool wait, int pid);

void child(char **lexered);

void showJobs();

void addJob(char *name, int pid);

void removeJob(char *name, int pid);

void orderJobs();

void cdFunc();

int main(int argc, char **argv) {
    jobPos = 0;
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
    } else if (pid > 0) {
        parent(isWait, pid);
    } else if (pid == 0) {
        printf("child\n");
        child(lexered);
        printf("endOfChild\n");
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
    addJob(lexered[0], getpid());
    if (strcmp(lexered[0], "jobs") == 0) {
        printf("JOBS!!\n");
        removeJob(lexered[0], getpid());
        showJobs();
        //  exit(4);
    } else if (strcmp(lexered[0], "cd") == 0) {
        cdFunc();
    } else if (execvp(lexered[0], lexered) == -1) {
        perror("ERROR IN EXE");
    }
    removeJob(lexered[0], getpid());
    //exit(3);
}

void showJobs() {
    printf("showJobs\n");
    orderJobs();
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("%d)%d %s\n", i, jobs[i].pid, jobs[i].name);
        }
    }
    printf("endOfShowJobs\n");
}

void addJob(char *name, int pid) {
    if (jobPos < MAX_JOBS) {
        jobs[jobPos].name = strdup(name);
        jobs[jobPos].pid = pid;
        jobPos++;
        printf("JOB created in %d\n", jobPos);
    } else {
        orderJobs();
        addJob(name, pid);
    }
}

void removeJob(char *name, int pid) {
    return;
    printf("REMOVE\n");
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid = pid) {
            jobs[i].name = "";
            jobs[i].pid = 0;
            printf("remove-V\n");
            return;
        }
    }
    printf("There isnt job to remove\n");
}

void orderJobs() {
    return;
    printf("orderJobs\n");
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        if ((strcmp(jobs[i].name, "") == 0) && (strcmp(jobs[i + 1].name, "") != 0)) {
            strcpy(jobs[i].name, jobs[i + 1].name);
            strcpy(jobs[i + 1].name, "");

            jobs[i].pid = jobs[i + 1].pid;
            jobs[i + 1].pid = 0;

            jobPos = i + 1;
        }
    }
    printf("EndOfOrder\n");
}

void cdFunc() {
    printf("CD\n");//todo
}