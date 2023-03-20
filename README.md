# Systems-Programming
Coding assignments I did for Systems Programming in C


## **PA1 -- mymalloc**
Implementing my own version of malloc and free. Open file for further details.\
 \
*Things I could have done better*: The metanode was 24 bytes which takes up a lot of space. I could have comfortably brought down the metanode size without changing the code up too much to 10 bytes which is half the length of the original I attached. I could change trackfree to a char instead of an int and remove the prev pointer and just have a trailing pointer within my code. /
*Notes after grades*: Do not use the physical number 4096! Use the macro defined in the header file.
