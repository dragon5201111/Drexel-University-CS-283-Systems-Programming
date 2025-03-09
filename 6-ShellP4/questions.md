1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client determines when a command's output is fully received from the server when it recieves an EOF character. The techniques that can be used to handle partial reads, where the data may not arrive all at once, the client handles this by reading data in chunks until the entire message is received. This is done by using a loop that keeps calling the read() or recv() function until the end of the message is reached, checking for the EOF marker or the closure of the connection.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

The networked shell protocol should implement techniques like delimiters or fixed message sizes to clearly demarcate the beginning and end of commands, ensuring reliable and accurate message transmission.The challenges that arise if this is not handled correctly the shell may fail to properly execute commands or incomplete commands could be processed.

3. Describe the general differences between stateful and stateless protocols.

A stateless protocol is a type of communication that doesn’t depend on previous communications between computers. In other words, stateless protocols don’t keep track of any information about the packets being sent. A stateful protocol keeps track of all the traffic between two communicating computers, it remembers what has happened to each packet so that they can quickly retransmit the packets in the event of a network failure.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

We use UDP because it is preferred when speed is more important than guaranteed delivery, such as when you are gaming over a network or using a video chat service. UDP provides faster transmission because it doesn't have the burden of establishing connections or ensuring reliability like TCP does.


5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The socket API (sockets).