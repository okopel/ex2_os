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


#define MAX_CHARS 20

int main() {
    char task[MAX_CHARS];
    printf("enter task\n");
    scanf("%s", &task);
    system(task);


    printf("end\n");


//    scanf("%s", &task);

    return 0;
}