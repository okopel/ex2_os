/**
 * Ex2
 * Ori Kopel
 * 205533151
 * 3/2019
 */

#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHARS 256
#define MAX_WORD 32
#define DELIMITER " "
#define WAITING_DELIMITER '&'
#define MAX_JOBS 512
struct Task {
    char *name;
    int pid;
};

struct Task jobs[MAX_JOBS];

int pos = MAX_JOBS;

void runShell();

char *getInput();

char **getLexer(char *input);

bool sendToExe(int index, char **lexered);

bool waitingCheck(char **lexered);

void waitToChild(int pid);

void dontWait(char *name, int pid);

void parent(int index, bool wait, char *name, int pid);

void child(char **lexered);

void showJobs();

void addJob(int jobPos, char *name, int pid);

void removeJob(char *name, int pid);

int orderJobs();

void cdFunc(char *path);

void exitFunc();

void manFunc(char **lexered);


int main(int argc, char **argv) {
    runShell();
    return 0;
}

void runShell() {
    char *input;
    char **lexered;
    bool hasNext = true;
    int jobPos = 0;
    while (hasNext) {
        input = getInput();
        lexered = getLexer(input);
        hasNext = sendToExe(jobPos, lexered);
        free(input);
        free(lexered);
        jobPos++;
    }
}

char *getInput() {
    char *input = malloc(sizeof(char) * MAX_CHARS);
    int maxLen = MAX_CHARS;
    if (!input) {
        perror("BAD MALLOC");
    }
    char c;
    int posi = 0;
    printf(">");
    while (true) {
        c = getchar();
        if (c == '\n' || c == EOF) {
            input[posi] = '\0';
            return input;
        } else {
            input[posi] = c;
            posi++;
        }
        if (posi == maxLen) {
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
    int posi = 0;
    char *tmp = strtok(input, DELIMITER);

    while (tmp) {
        lexered[posi] = tmp;
        posi++;
        if (posi == maxWord) {
            maxWord += MAX_WORD;
            lexered = realloc(lexered, maxWord);
            if (!lexered) {
                perror("BAD REALLOC IN LEXER");
            }
        }
        tmp = strtok(NULL, DELIMITER);
    }
    lexered[posi] = NULL;

    return lexered;
}

bool sendToExe(int index, char **lexered) {
    if (lexered[0] == NULL) {
        return true;
    }
    bool isWait = waitingCheck(lexered);
    if (strcmp(lexered[0], "jobs") == 0) {
        showJobs();
        return true;
    } else if (strcmp(lexered[0], "exit") == 0) {
        return false;
    }

    pid_t pid;
    pid = fork();
    if (pid == -1) {//Error case
        perror("ERROR IN FORK\n");
        exit(1);
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

void dontWait(char *name, int pid) {
    printf("DONT-WAIT-CASE %d\n", pid);

}

void waitToChild(int pid) {
    int status;
    do {
        waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void parent(int index, bool wait, char *name, int pid) {
    addJob(index, strdup(name), pid);
    printf("%d\n", pid);
    if (!wait) {
        waitToChild(pid);
        removeJob(strdup(name), pid);
    } else {
        // dontWait(name, pid);
    }
}

void child(char **lexered) {
    if (strcmp(lexered[0], "cd") == 0) {
        if (lexered[1] != NULL) {
            cdFunc(lexered[1]);
        }

    } else if (strcmp(lexered[0], "exit") == 0) {
        exitFunc();
    } else if (strcmp(lexered[0], "man") == 0) {
        printf("MAN\n");
        manFunc(lexered);
        printf("MAN\n");

    } else if (execvp(lexered[0], lexered) == -1) {
        fprintf(stderr, "Error in system call\n");
    }
}

void showJobs() {
    for (int i = 0; i < pos; i++) {
        if (jobs[i].name == NULL || jobs[i].pid == 0) {
            continue;
        }
        if (waitpid(jobs[i].pid, NULL, WNOHANG != 0)) {
            removeJob(jobs[i].name, jobs[i].pid);
            continue;
        }
        if ((jobs[i].pid != 0) && (jobs[i].name != NULL)) {
            printf("%d %s\n", jobs[i].pid, jobs[i].name);
        }
    }
}

void addJob(int jobPos, char *name, int pid) {
    if (jobPos < pos) {
        jobs[jobPos].name = strdup(name);
        jobs[jobPos].pid = pid;
    } else {
        int newJobPos = orderJobs();
        printf("%d\n", newJobPos);
        addJob(newJobPos, strdup(name), pid);
    }
}

void removeJob(char *name, int pid) {
    for (int i = 0; i < pos; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        //  printf("i:%d)~\tis eq:%d~\t(pid:%d,jobPid:%d)\n", i, jobs[i].pid == pid, pid, jobs[i].pid);
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
    for (int i = 0; i < pos; i++) {
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
    int jobIndex = pos;
    while (jobIndex > 0) {
        printf("!");
        if (jobs[jobIndex].name != NULL) {
            return jobIndex + 1;
        }
        jobIndex--;
    }
    printf("EndOfOrder\n");
}

void cdFunc(char *path) {
    int ret = chdir(path);
    if (ret) {
        fprintf(stderr, "Error in system call\n");
    }
}

void exitFunc() {
    exit(1);
}

void manFunc(char **lexered) {
    printf("d\n");
    char **newLex;
    int x = 0, i = 1;
    newLex[x++] = strdup("man");
    newLex[x++] = strdup("-P");
    newLex[x++] = strdup("cat");
    while (lexered[i] != NULL) {
        newLex[x++] = strdup(lexered[i++]);
    }
    printf("!%s! %s! %s!", newLex[0], newLex[1], newLex[2]);
    if (execvp(newLex[0], newLex) == -1) {
        fprintf(stderr, "Error in system call\n");
    }

}