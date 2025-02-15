1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We use fork/execvp instead of directly calling execvp because,  if we didn't the entire memory footprint of our shell would be replaced with whatever is to be ran by execvp. The fork provides a copy of a parent process, in our case, the shell, which is valuable because now we have a process that can be overwritten by another process our shell wants to run.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork system call fails, errno is set and no child process is created. My implementation checks if fork fails and returns errno to the main execution context to be handled elsewhere.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp uses the PATH environment variable to find out where a command is located.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of callling wait is to suspend the parent process's execution until the child process it created has finished running, allowing the parent to retrieve the exit status of the child process before continuing its own execution. If we didn't wait, the parent process could potentially finish execution before the child process creating a zombie process, wasting system resources.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  _start here_

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  _start here_

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  _start here_

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  _start here_

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  _start here_

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  _start here_
