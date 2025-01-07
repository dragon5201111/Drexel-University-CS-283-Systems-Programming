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
int reverse_buff(char **, int);
int print_reversed_words(char *, int, int);
int print_words(char *, int, int);

int setup_buff(char *buff, char *user_str, int len){
    // Buffer or user_str is empty.
    if(buff == NULL || user_str == NULL){
        printf("setup_buff requires a non-null buffer and user_str.\n");
        return -2;
    }

    if(len > BUFFER_SZ){
        printf("user_str too large.\n");
        return -1;
    }

    int was_space = 0, j = 0;

    char current_char;

    for(int i = 0; i < len && *(user_str + i); i++){
        current_char = *(user_str + i);

        if (isspace(current_char)) {
            if (!was_space) {
                *(buff + j) = current_char;
                j++;
            }
            was_space = 1;
        } else {
            *(buff + j) = current_char;
            j++;
            was_space = 0;
        }
    }

    if(j < BUFFER_SZ){
        memset(buff + j, '.', BUFFER_SZ - j);
    }

    return j; 
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
    // Empty input buffer
    if(buff == NULL){
        printf("Buffer cannot be empty.\n");
        return -2;
    }

    if(str_len > len){
        printf("String length cannot exceed buffer length.\n");
        return -1;
    }

    int word_count = 0, in_word = 0;

    char current_char;

    for (int i = 0; i < str_len; i++)
    {
        current_char = *(buff + i);

        if(!isspace(current_char)){
            if(!in_word){
                word_count++;
                in_word = 1;
            }
        }else{
            in_word = 0;
        }
    }
    
    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int reverse_buff(char ** buff, int len){
    // Empty input buffer
    if(*buff == NULL || buff == NULL){
        printf("Buffer cannot be empty.\n");
        return -2;
    }

    char * new_buffer = (char *) malloc(sizeof(char) * len);

    // Error allocating for buffer to hold reversed characters.
    if(new_buffer == NULL){
        printf("Error allocating for new buffer.\n");
        return -2;
    }


    for (int i = 0, j = len - 1; j >= 0; i++, j--) {
        *(new_buffer + i) = *(*buff + j);
    }

    // Free old buffer
    free(*buff);
    *buff = new_buffer;

    return 0;
}

int print_reversed_words(char * buff, int len, int str_len){
    if(buff == NULL) return -2;
    if(len < str_len || str_len < 0) return -1;

    printf("Reversed String: ");
    for(int i = (len - str_len); i < len; i++){
        putchar(*(buff + i));
    }
    putchar('\n');

    return 0;
}

int print_words(char * buff, int len, int str_len){
    if(buff == NULL) return -2;
    if(len < str_len || str_len < 0) return -1;

    printf("Word Print\n");
    printf("----------\n");

    int line_count = 0, word_len = 0, in_word = 0;

    char current_char;
    
    for (int i = 0; i < str_len; i++)
    {
        current_char = *(buff + i);

        if(!isspace(current_char)){
            if(!in_word){
                line_count++;
                in_word = 1;
                word_len = 1;
                printf("%d. ", line_count);
            }else{
                word_len++;
            }
            putchar(current_char);
        }else{
            if (in_word) {
                printf(" (%d)\n", word_len);
                in_word = 0;
            }
        }
    }

    if (in_word) {
        printf(" (%d)\n", word_len);
    }

    return 0;
}


int main(int argc, char *argv[]){

    char *buff; //placeholder for the internal buffer

    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    /*
        This is safe because argc is checked to see if it is less than two first.
        This ensures that there are at least two arguments present first before checking argv[1].
        If argc is less than two, argv[1] would be out of bounds, and accessing it would cause undefined behavior.
        But because the  pricheck occurs first, it is safe.
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


    //TODO:  #2 Document the purpose of the if statement below
    /*
        All other flags beyond the -h flag require an argument(s). It is necessary to check for
        them before continuing the program.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
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
        
        case 'r':
            rc = reverse_buff(&buff, BUFFER_SZ);
            if (rc < 0){
                printf("Error reversing words, rc = %d", rc);
                exit(2);
            }
            rc = print_reversed_words(buff, BUFFER_SZ, user_str_len);

            if (rc < 0){
                printf("Error printing reversed words, rc = %d", rc);
                exit(3);
            }
            break;
        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(3);
            }
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
//          Bounds checking. C doesn't know the extent of a pointer. For all C knows, the buffer goes on infinitely.
//          By passing the buffer size, eliminates guessing and helps to ensure that memory outside bounds is not accessed,
//          which can lead to undefined behavior and buffer overflows.