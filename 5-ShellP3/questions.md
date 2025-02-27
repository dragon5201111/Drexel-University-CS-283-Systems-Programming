1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation forks a supervisor, and then calls wait waitpid() on all children that the supervisor forks. Afterwards, waitpid is called on the supervisor. If you forgot to call waitpid on the child processes, control would return back to the parent processes before the children have an opportunity to finish their work, and the children would become zombie processes (existing in the process table).

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Each process has a limited amount of file descriptors. It's necessary to close the unused pipe ends to prevent resource leaks. If a process leaves unused pipe ends open, it can cause issues like running out of avaliable file descriptors; leading to a situation where the process can no longer open new files or pipes because it has run out of available file descriptors.

When you call dup2, the new_fd points to the old_fd, but you should close old_fd because it is referenced by new_fd and the old_fd is taking up avaliable file descriptors. Which can quickly be exhausted if you are creating lots of pipes within a single process.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is implemented as a built-in command because it directly modifies the current working directory of the shell process. If cd were an external command, it would run in a separate child process, which could only change the working directory for that child process and not affect the parent shell.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would modify my implementation by changing the command_list_t structure to contain a cmd_buff_t * instead of a statically sized array. To do this you would need to consider the fact you have a variable number of piped commands, so dynamic memory allocation would be required to manage the growing list. I would implement a llinked-list to store the commands, ensuring that memory is allocated and deallocated as needed. This would allow flexibility in supporting an arbitrary number of piped commands. However, you'd need to handle reallocations and ensure proper memory cleanup to avoid memory leaks. Additionally, the dynamic memory allocation could introduce slight performance overhead compared to a fixed-size array, especially when reallocating or managing memory for a large number of commands.
