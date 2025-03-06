#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int exitSuperVisor;
    pid_t supervisor;
    printf("[p] Main parent pid:%d\n", getpid());

    if ((supervisor = fork()) == -1) {
        perror("Couldn't fork supervisor.\n");
        exit(EXIT_FAILURE);
    }

    if (supervisor == 0) {  // Supervisor process
        pid_t childOne, childTwo;
        int exitChildOne, exitChildTwo;
        int pipeFd[2];

        // pipeFd[1] = write end
        // pipeFd[0] = read end

        if (pipe(pipeFd) == -1) {
            perror("Couldn't create pipe in supervisor.\n");
            exit(EXIT_FAILURE);
        }

        // Create first child
        if ((childOne = fork()) == -1) {
            perror("Error forking child one of supervisor.\n");
            exit(EXIT_FAILURE);
        }

        if (childOne == 0) {  // First child
            putchar('\n');

            char * write = (argv[1] == NULL) ? "hello" : argv[1];
            printf("[c] Child of pid:%d writing '%s' to stdout ...\n", getpid(), write);

            // Close read end of the pipe (not needed in child one)
            close(pipeFd[0]);
            dup2(pipeFd[1], STDOUT_FILENO);

            printf("%s", write);

            close(pipeFd[1]);
            
            
            exit(EXIT_SUCCESS);
        }

        // Create second child
        if ((childTwo = fork()) == -1) {
            perror("Error forking child two of supervisor.\n");
            exit(EXIT_FAILURE);
        }

        if (childTwo == 0) {  // Second child
            putchar('\n');

            printf("[c] Child of pid:%d reading from stdin ...\n", getpid());

            // Close write end of the pipe (not needed in child two)
            close(pipeFd[1]);
            // Redirect stdin to the read end of the pipe
            dup2(pipeFd[0], STDIN_FILENO);

            printf("[c] Child of pid:%d read: ", getpid());
            
            int c;

            while((c = getchar()) != EOF){
                putchar(c);
            }

            putchar('\n');

            close(pipeFd[0]);

            exit(EXIT_SUCCESS);
        }

        // In supervisor: close both ends of the pipe
        close(pipeFd[0]);
        close(pipeFd[1]);

        // Wait for child processes to finish
        waitpid(childOne, &exitChildOne, 0);
        waitpid(childTwo, &exitChildTwo, 0);

        putchar('\n');

        printf("[s] Child one exited with:%d, Child two exited with:%d\n", exitChildOne, exitChildTwo);
        exit(EXIT_SUCCESS);
    }

    // In parent: wait for supervisor process
    printf("[p] Supervisor pid:%d\n", supervisor);
    waitpid(supervisor, &exitSuperVisor, 0);

    putchar('\n');

    printf("[p] Supervisor exited with:%d\n", exitSuperVisor);

    return 0;
}
