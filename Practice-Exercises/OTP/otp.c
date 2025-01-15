#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARG_DELIM "="
#define IN "in"
#define OUT "out"

int setInOut(char **, char **, char **);
int processArg(char *, char *, char **);
char * createCipherText(char *, char *, int, int);
char * createKey(int);

int main(int argc, char * argv[]) {
    srand(time(NULL)); // Set seed for random to be time since Unix epoch
    
    if(argc <= 1 || argc > 3){
        return 1;
    }

    char * in;
    char * out;

    if(setInOut(argv, &in, &out) < 0){
        printf("Unable to set in & out.\n");
        return 1;
    }

    printf("%s %s\n", in, out);

    return 0;
}


int setInOut(char ** args, char ** in, char ** out){
    int ret1= processArg(args[1], IN, in);
    int ret2 = processArg(args[2], OUT, out);

    if(ret1 < 0 || ret2 < 0) return -1;

    return 0;
}


int processArg(char * arg, char * keyword, char ** result){
    char *token;

    token = strtok(arg, ARG_DELIM);
    
    if (token == NULL) {
        return -1;
    }

    if (strcmp(token, keyword) != 0) {
        return -2;
    }

    token = strtok(NULL, ARG_DELIM);

    if (token == NULL) {
        return -1;
    }

    *result = (char *) malloc(sizeof(char) * (strlen(token) + 1));

    if (*result == NULL) {
        return -1;
    }

    strcpy(*result, token);
    return 0;
    
}


char * createCipherText(char * text, char * key, int textSize, int keySize) {
    if (keySize <= 0 || textSize <= 0) return NULL;
    if (keySize != textSize) return NULL;

    char * cipherText = (char *) malloc(sizeof(char) * (textSize + 1));

    if (cipherText == NULL) return NULL;

    for (int i = 0; i < textSize; i++) {
        cipherText[i] = text[i] ^ key[i];
    }

    cipherText[textSize] = '\0';

    return cipherText;
}

char * createKey(int size) {
    if (size + 1 <= 0) return NULL;

    char * key = (char *) malloc(sizeof(char) * (size + 1));

    if (key == NULL) return NULL;

    for (int i = 0; i < size; i++) {
        int randomIndex = rand() % 62;
        if (randomIndex < 10) {
            key[i] = '0' + randomIndex;
        } else if (randomIndex < 36) {
            key[i] = 'a' + randomIndex - 10;
        } else {
            key[i] = 'A' + randomIndex - 36;
        }
    }

    key[size] = '\0';

    return key;
}
