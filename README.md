# Systems-Programming
Coding assignments I did for Systems Programming in C (214)

## **PA1 -- mymalloc**
Implementing my own version of malloc and free. Open file for further details.\
 \
*Things I could have done better*: The metanode was 24 bytes which takes up a lot of space. I could have comfortably brought down the metanode size without changing the code up too much to 10 bytes which is half the length of the original I attached. I could change trackfree to a char instead of an int and remove the prev pointer and just have a trailing pointer within my code. 

## **PA2 -- myshell**
Implementing my own version of shell to run in intereactive and batch mode.\
\
*Things I could have done better*: Start the assignment earlier! This is assignment is a pain in the ass and is slightly broken with bandaid fixes. I will come back to it with better fixes.

## **PA3 -- Tic-Tact_Toe On-Line**
Implementing a tic-tac-toe game to run on different clients given a domain and port number.\
\
*Things I could have done better*: Implement multithreading to its entirety. The assignment was cut short due to the strike.
