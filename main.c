/**
 * Ex2 in OS course
 * Name: Ori Kopel
 * kopelOr in u2
 * ID 205533151
 * 3/2019
 */

#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHARS 512
#define MAX_WORD 512
#define DELIMITER " "
#define WAITING_DELIMITER '&'
#define MAX_JOBS 512

struct Task {
    char *name;
    int pid;
};

struct Task jobs[MAX_JOBS];

int pos = MAX_JOBS;
char PATH[MAX_CHARS];

/**
 * Running the shell
 */
void runShell();

/**
 * @return one string from the user.
 */
char *getInput();

/**
 * parse the input to an array
 * @param input the string from the user
 * @return an array
 */
char **getLexer(char *input);

/**
 * execute the functions
 * @param index of our function
 * @param lexerd the array
 * @return true if there is another function
 */
bool sendToExe(int index, char *input, char **lexered);

/**
 * check for '&' - a sign of waiting and delete him.
 * @param lexered the array
 * @return true if we should wait.
 */
bool waitingCheck(char **lexered);

/**
 * the waiting process
 * @param pid of child
 */
void waitToChild(int pid);

/**
 * the parent process
 * @param index our function
 * @param wait should we wait to child?
 * @param name func name
 * @param pid of child
 */
void parent(int index, bool wait, char *input, int pid);

/**
 * child process
 * @param lexered the array
 */
void child(char **lexered);

/**
 * list of running jobs.
 */
void showJobs();

/**
 * add jobs to list
 * @param jobPos position
 * @param name of job
 * @param pid of child who run the job
 */
void addJob(int jobPos, char *name, int pid);

/**
 * remove job from list
 * @param name to remove
 * @param pid to remove
 */
void removeJob(int pid);

/**
 * order the jobs after some deletes.
 * @return new position after ordering.
 */
int orderJobs();

/**
 * CD func
 * @param path the path to CD
 */
void cdFunc(char *path);

/**
 *  Reparse the path to CD
 * @param path the old path
 * @return the new path witouht ""
 */
char *reParse(char *path);

/**
 * delete CD from the path
 * @param path the path
 */
char *deleteCD(char *path);

/**
 * Exit func
 */
void exitFunc();


/**
 * MAIN
 * @return 1 for success
 */
int main() {
    getcwd(PATH, sizeof(PATH));
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
        lexered = getLexer(strdup(input));
        hasNext = sendToExe(jobPos, input, lexered);
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
    printf("> ");
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

bool sendToExe(int index, char *input, char **lexered) {
    if (lexered[0] == NULL) {
        return true;
    }
    bool isWait = waitingCheck(lexered);
    if (strcmp(lexered[0], "jobs") == 0) {
        showJobs();
        return true;
    } else if (strcmp(lexered[0], "exit") == 0) {
        return false;
    } else if (strcmp(lexered[0], "cd") == 0) {
        cdFunc(input);
        return true;

    }
    pid_t pid;
    pid = fork();
    if (pid == -1) {//Error case
        perror("ERROR IN FORK\n");
        exit(1);
    } else if (pid > 0) {
        parent(index, isWait, strdup(input), pid);
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
        if (strcmp(lexered[i - 1], "\0") == 0) {//case the & after space
            lexered[i - 1] = NULL;
        }
        return true;
    }
    return false;
}

void waitToChild(int pid) {
    int status;
    do {
        waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void parent(int index, bool wait, char *input, int pid) {
    if (wait) {//delete the &
        input[strlen(input) - 1] = '\0';
    }
    addJob(index, strdup(input), pid);
    printf("%d\n", pid);
    if (!wait) {
        waitToChild(pid);
        removeJob(pid);
    }
}

void child(char **lexered) {
    if (strcmp(lexered[0], "exit") == 0) {
        exitFunc();
    } else if (execvp(lexered[0], lexered) == -1) {
        fprintf(stderr, "Error in system call\n");
    }
}

void showJobs() {
    int i;
    for (i = 0; i < pos; i++) {
        if (jobs[i].name == NULL || jobs[i].pid == 0) {
            continue;
        }
        if (waitpid(jobs[i].pid, NULL, WNOHANG != 0)) {
            removeJob(jobs[i].pid);
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
        addJob(newJobPos, name, pid);
    }
}

void removeJob(int pid) {
    int i;
    for (i = 0; i < pos; i++) {
        if (jobs[i].name == NULL) {
            continue;
        }
        if (jobs[i].pid == pid) {
            jobs[i].name = "";
            jobs[i].pid = 0;
            return;
        }
    }
}

int orderJobs() {

    int i;
    for (i = 0; i < pos; i++) {
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

    int jobIndex = pos;
    while (jobIndex > 0) {
        if (jobs[jobIndex].name != NULL) {
            return jobIndex + 1;
        }
        jobIndex--;
    }
}

void cdFunc(char *path) {
    path = deleteCD(path);
    if (path[0] == '-') {
        char c[MAX_CHARS];
        getcwd(c, MAX_CHARS);
        chdir(PATH);
        strcpy(PATH, c);

        return;
    } else if ((path == NULL) || strlen(path) == 0 || (strcmp(path, "~") == 0)) {
        getcwd(PATH, sizeof(PATH));
        chdir(getenv("HOME"));
        return;
    }
    getcwd(PATH, sizeof(PATH));
    path = reParse(path);
    int ret = chdir(path);
    if (ret) {
        fprintf(stderr, "Error in system call\n");
    }
}

char *deleteCD(char *path) {
    int i = 0;
    while (path[i] == ' ') {//delete spaces
        path++;
    }
    if (path[0] == 'c') {//delete cd
        path += 2;
    }
    i = 0;
    while (path[i] == ' ') {
        path++;
    }
    return path;
}

char *reParse(char *path) {
    //printf(":%s\n", path);
    if (path[0] == '"') {
        path++;
        int i = 0;
        while (path[i] != '"') {
            i++;
            if (i == MAX_CHARS) {
                break;
            }
        }
        path[i] = '\0';
    }
    //printf("::%s\n", path);
    if (strlen(path) == 0 || path[0] == '\0') {
        return NULL;
    }
    return path;
}

void exitFunc() {
    exit(1);
}