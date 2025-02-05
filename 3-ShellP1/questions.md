1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets is a good choice for this application because it reads characters from a specified stream. Since this application is a shell, it works nicely since the application will need to read user-input from the stdin stream. Also, fgets stops reading after an EOF or a newline - which makes it essentially a "line by line" processor. This is important for shells.

2. You needed to use `malloc()` to allocate memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Malloc was used for the cmd_buff instead of a static array because, the cmd_buff size might need to change in the future. A dynamic array gives us this flexibility whereas a fixed-size array does not. For example, we can resize the cmd_buff.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: Leading and trailing whitespace must be trimmed because if it not, the whitespace might lead to misinterpretation when executing a commmand. For example, suppose an ls command is issued with one space after it. Is the space an argument to the ls command? Removing whitespace from the head and tail of a command removes ambiguity.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  
        Example 1: command1 > command2\
        Example 2: command1 < command2\
        Example 3: command1 >> command2\
        \
         For example 1, we may have challenges if command1 is not accessible or if we don't have permissions to write to command2. For example 2, we may have some challenges if command2 does not exist or command 1 does not exist. Finally, for example 3, we face the same challenges as example 1 however now we are concerned with appending to command2. Overall with all 3 examples, we face more or less the same challenges: File existence, File permissions and Logic.
        

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  A pipe is used to pass output of one program to the input of another. In other words, to connect the STDOUT of one command to the STDIN of another. Whereas redirection reroutes a command's input or output to or from a file.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: Keeping STDERR and STDOUT seperate is important because it becomes easier to distinguish between normal program results and issues that need attention. If both are mixed, it becomes difficult to spot errors in the output. Also, it allows users to redirect them independently. For example, you could redirect normal output to a file while sending errors to a log file or displaying them on the screen. This is particularly useful in scripts and when debugging, as you can store output and errors separately for analysis later.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  We should provide a way to merge STDOUT and STDERR. We can achieve this via  shell feature. Perhaps, a user issues a command with 2>&1 (like Linux) which signals to merge STDERR and STDOUT. We can achieve handling errors from commands that fail by implementing an exit status of the most recent command and printing the command's output to STDOUT and STDERR by default.