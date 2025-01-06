#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50
#define isspace(c) (c == ' ' || c == '\t') // Macro function

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    if(buff == NULL || user_str == NULL){
        printf("setup_buff requires a non-null buffer and user_str.\n");
        return -2;
    }

    if(len > BUFFER_SZ){
        printf("user_str too large.\n");
        return -1;
    }

    int wasSpace = 0;
    int j = 0;

     char currentChar;

    for(int i = 0; i < len && *(user_str + i); i++){
        currentChar = *(user_str + i);

        if (isspace(currentChar)) {
            if (!wasSpace) {
                *(buff + j) = currentChar;
                j++;
            }
            wasSpace = 1;
        } else {
            *(buff + j) = currentChar;
            j++;
            wasSpace = 0;
        }
    }

    if(j < BUFFER_SZ){
        memset(buff + j, '.', BUFFER_SZ - j);
    }

    *(buff + j) = '\0';
    return j; //for now just so the code compiles. 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    return 0;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff; //placeholder for the internal buffer

    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    /*
        This is safe because argc is checked to see if it is less than two first.
        This ensures that there are at least two arguments present first before checking argv[1].
        If argc is less than two, argv[1] would be out of bounds, and accessing it would cause undefined behavior.
        But because the check occurs first, it is safe.
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    /*
        All other flags beyond the -h flag require an argument(s). It is necessary to check for
        them before continuing the program.
    */

    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    buff = (char *) malloc(sizeof(char) * BUFFER_SZ);

    if(buff == NULL){
        printf("Buffer memory allocation failed.\n");
        return 99;
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos

    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        default:
            usage(argv[0]);
            exit(1);
    }

    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE