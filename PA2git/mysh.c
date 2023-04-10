/*
mysh has both interactive and batch modes
for full credit, must have one input loop and command parsinga algo that
works for both modes

Goal of assignment
•Posix (unbuffered) stream IO
•Reading and changing the working directory
•Spawning child processes and obtaining their exit status
•Use of dup2() and pipe()
•Reading the contents of a directory

Give one argument (file name)-> BATCH MODE
Given no arguments -> INTERACTIVE MODE



Note : alot of reading input line was taken from 
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>

#ifndef BUFFSIZE
#define BUFFSIZE 1024
#endif

#ifndef DEBUG
#define DEBUG 1
#endif

int interative=0;

int failedcommand=0;
//pointer array to strings 
char *linebuffer;
int linepos; 
int linesize;
int waiting =1;
int pwdwait;
int inputRedirect = 0;
int filno;
void outRedirect(char *tokenized);
void inRedirect(char *tokenized);
void get_token();
void pwd();

int exitsh()
{
    write(STDOUT_FILENO,"mysh: exiting\n",14);
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

void pwd() 
{
    char dir[256];
    if (getcwd(dir, sizeof(dir)) != NULL) 
    {
        write(STDOUT_FILENO,dir,strlen(dir));
        write(STDOUT_FILENO, "\n",1);
        if (interative == 1) 
        {
            write(STDIN_FILENO,"mysh> ",6);
        }
        fflush(stdout);
    } 
    else 
    {
        perror("Error: Could not get current directory.");
        failedcommand = 1;
    }
}

////////////// CD FUNCTION ////////////////
void cd_traversal(char* path) 
{
    if (path == NULL) 
    { // No arguments provided, change directory to home directory
        if (getenv("HOME") == NULL) 
        {
            perror("cd");
            failedcommand = 1;
            return;
        }
        if (chdir(getenv("HOME")) == -1) 
        {
            perror("cd");
            failedcommand = 1;
        }
        return;
    }
    if (path[0] == '~') 
    { //Given a ~/ argument
        char* homePath = getenv("HOME");
        if (getenv("HOME") == NULL) 
        {
            perror("cd");
            failedcommand = 1;
            return;
        }
        char* fullPath = malloc(strlen(homePath) + strlen(path));
        if (fullPath == NULL) 
        {
            write(STDOUT_FILENO,"cd: failed to allocate memory", 30);
            failedcommand = 1;
            return;
        }
        strcpy(fullPath, homePath);
        strcat(fullPath, path + 1);
        if (chdir(fullPath) == -1) 
        {
            failedcommand = 1;
            perror("cd");
        }
        free(fullPath);
    } 
    else 
    { //Given a filepath
        if (chdir(path) == -1) 
        {
            failedcommand = 1;
            perror("cd");
        }
    }
}

void inRedirect(char *filename)
{
    char *file = strtok_r(NULL," \t\n", &filename);
    int input = open(file, O_RDONLY);
    inputRedirect = 1;
        if (strstr((const char*) file, "*") != NULL)
    {
        printf("Invalid command: cannot do wildcard\n");
        failedcommand = 1;
        //return EXIT_FAILURE;
    }
    if (input==-1)
    {
        perror("Cannot open file");
        failedcommand=1;
    }
    dup2(input, STDIN_FILENO);
    close(input);
}

void outRedirect(char *filename)
{
    char *file = strtok_r(NULL," \t\n", &filename);
    int output = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0640);
     if (strstr((const char*) file, "*") != NULL)
    {
        printf("Invalid command: cannot do wildcard\n");
        failedcommand = 1;
        //return EXIT_FAILURE;
    }
    if (output==-1)
    {
        perror("cannot open file");
        failedcommand=1;
    }
    dup2(output, STDOUT_FILENO);
    close(output);
}

void wildcards(char *filename)
{
    glob_t glob_result;
    puts(filename);
    int ret = glob(filename, 0, NULL, &glob_result);
    if (ret == 0) 
    {
        printf("Matched files:\n");
        if (glob_result.gl_pathc>0)
        {
        //for (int i = 0; i < glob_result.gl_pathc; i++) {
            //printf("%s\n", glob_result.gl_pathv[i]);
        }
    } 
    else 
    {
        write(STDOUT_FILENO, "Error: No Match Found\n", 23);
        fflush(stdout);
    }
}

int piping(char *firstCommand, char *secondCommand) //firstCommand | secondCommand
{
    //printf("\nThis is the line I passed %s|%s\n",firstCommand,secondCommand); 

    //hard coding error checking for invalid pipe command
    if (strstr((const char*) secondCommand, "cd") != NULL)
    {
        printf("Invalid command: cannot do __ | cd\n");
        failedcommand = 1;
        return EXIT_FAILURE;
    }    if (strstr((const char*) secondCommand, "*") != NULL)
    {
        printf("Invalid command: cannot do wildcards\n");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
        if (strstr((const char*) firstCommand, "*") != NULL)
    {
        printf("Invalid command: cannot do wildcards\n");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    //handle hard coding to ensure single input output stream
    if (strstr((const char*) firstCommand, ">") != NULL)
    {
        printf("Invalid command: more than one output source specified\n");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    if (strstr((const char*) secondCommand, "<") != NULL)
    {
        printf("Invalid command: more than one input source specified\n");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    //handle actual redirection here- ahmed check this out pls
    strtok(firstCommand, "<");
    char *firstPTwo = strtok(NULL, "");
    inRedirect(firstPTwo);
    // printf("firstP: %s, firstPTwo: %s\n", firstP, firstPTwo);
    
    strtok(secondCommand, ">"); 
    char *secPTwo = strtok(NULL, "");
    outRedirect(secPTwo);
    // printf("secP: %s, secPTwo: %s\n", secP, secPTwo);

    int fp[2];
    int firstChild, secondChild; //need 2 child processes for foo | bar, 3 child processes if foo | bar | baz
    int childID;
    int status;
    char *childName;
    //fd[0] read from
    //fd[1] write to
    if (pipe(fp) == -1) //fork returns -1 when it fails
    {
        perror("pipe failure");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    firstChild = fork(); 
    if (firstChild == -1)
    {
        perror("first child fork failure");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    if (firstChild == 0) //in the child process before the |, like foo for example
    {
        close(fp[0]); //close the read end bc you just use child process to write to parent
        dup2(fp[1], STDOUT_FILENO); //child process writes to pipe
        close(fp[1]); //close fp1 now bc we r using stdoutfileno
        
        char *firstPart[500];
        int i = 0;
        char *firstToken = strtok(firstCommand, " ");
        while (firstToken != NULL && *firstToken != '<' && *firstToken != '>') 
        {
            firstPart[i] = firstToken;
            firstToken = strtok(NULL, " ");
            i++;
        }
        firstPart[i] = NULL;

        //get_token(firstCommand);
        execvp(firstPart[0], firstPart);
        //if execvp returns anything, we failed
        perror("second child exec call failure");
        failedcommand = 1;
        exit(EXIT_FAILURE); //is this part right?
    }
    //parent process, start the second child process for token after the | 
    secondChild = fork();
    if (secondChild == -1)
    {
        perror("second child fork failure");
        failedcommand = 1;
        return EXIT_FAILURE;
    }
    if (secondChild == 0) //in the child process after the |
    {
        close(fp[1]); //close write end bc you just read from child process
        dup2(fp[0], STDIN_FILENO); //child process reads from the first child process
        close(fp[0]); //we can do this now after dup2

        char *secPart[500];
        int p = 0;  
        char *secondToken = strtok(secondCommand, " ");
        while (secondToken != NULL && *secondToken != '<' && *secondToken != '>')
        {
            secPart[p] = secondToken;
            secondToken = strtok(NULL, " ");
            p++;
        }
        secPart[p] = NULL;

        execvp(secPart[0], secPart);
        //if exec returns, there was an error
        perror("second child exec call failure");
        failedcommand = 1;
        exit(EXIT_FAILURE);
    }

    close(fp[0]);
    close(fp[1]);
    for (int i = 0; i < 2; i++)
    {
        childID= wait(&status);
        //if childID == firstChild, expression evaluates to "firstChild",
        //if childID != firstChild, expression evaluates to "secondChild"
        childName = childID == firstChild ? "firstChild" : "secondChild"; 
        if (WIFEXITED(status)) 
        {
            //printf("%s exited with status %d\n", childName, WEXITSTATUS(status));
        }  
        else 
        {
            printf("%s exited abnormally\n", childName); //if exited bc of signalling
            failedcommand = 1;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS; //should this be EXIT_SUCCESS or exit(0) or something
}



///////////////GET TOKEN FUNCTION/////////////
//tokens are characters seperated by the whitespace and \,<, and >
void get_token()
{
    char *directOne = "/usr/local/sbin/";
 	char *directTwo = "/usr/local/bin/";
 	char *directThree = "/usr/sbin/";
 	char *directFour = "/usr/bin/";
 	char *directFive = "/sbin/";
 	char *directSix = "/usr/local/sbin/";
    char filepath1[500];
    char filepath2[500];
    char filepath3[500];
    char filepath4[500];
    char filepath5[500];
    char filepath6[500];

    
    //fprintf(stderr,"%s",linebuffer);
    char*tokenlist=malloc(BUFFSIZE);
    int l=0;
    int r= linepos+2;
    int i=0;
    while (linebuffer[i] == ' ' || linebuffer[i] == '\t') {
        i++;
    }
    if (strncmp(&linebuffer[i], "cd", 2) == 0) { 
        i += 2; 
        while (linebuffer[i] == ' ' || linebuffer[i] == '\t') {
            i++;
        }
        if (linebuffer[i] == '\n' || linebuffer[i] == '\0') {  
            cd_traversal(NULL); 
            free(tokenlist);
            return;
        } else { 
            char *dir = strtok(&linebuffer[i], " \t\n");
            cd_traversal(dir); 
            free(tokenlist);
            return;
        }
    }
    for (; i<r; i++)
    {
        if (linebuffer[i]!='>'&& linebuffer[i]!='<'&&linebuffer[i]!='|')
        {
            tokenlist[l++]=linebuffer[i];
        }
        else
        {
            tokenlist[l++]=' ';
            tokenlist[l++]=linebuffer[i];
            tokenlist[l++]=' ';
        }
 
    }
        tokenlist[l++]='\0';
        char *end;
        end=tokenlist+r+1;
        end--;
        *(end+1)='\0';
        char *args[BUFFSIZE];
        char *tokenized= strtok(tokenlist, " ");
        char *breakdown = malloc(strlen(tokenized) + 1);
        strcpy(breakdown, tokenized);
        breakdown = strtok_r(NULL," \t\n", &breakdown);
        sprintf(filepath1, "%s/%s", directOne, breakdown);
        sprintf(filepath2, "%s/%s", directTwo, breakdown);
        sprintf(filepath3, "%s/%s", directThree, breakdown);
        sprintf(filepath4, "%s/%s", directFour, breakdown);
        sprintf(filepath5, "%s/%s", directFive, breakdown);
        sprintf(filepath6, "%s/%s", directSix, breakdown);
        ///checking first bin

        //printf(stderr,"%s",filepath1);
        if (strcmp(breakdown, "pwd") == 0) {
            pwdwait=1;
            pwd();
            free(breakdown);
            free(tokenlist);
            return;
        } else if (strcmp(breakdown, "exit") == 0) {
            exitsh();
            free(breakdown);
            free(tokenlist);
            return;
        } 
        int parse=0;
        //printf("linebuffer currently contains: %s\n", linebuffer);
        //printf("*tokenized currently contains: %c\n", *tokenized);
        const char* linebufferCopy = (const char*) linebuffer; //make a constant char copy of the buffer
        if (strchr(linebufferCopy, '|') != NULL) //returns a ptr to the first instance or NULL
        {
            tokenized = strchr(linebufferCopy, '|');
           // printf("Pipe command: tokenized is %s\n", tokenized);
        }
        while (tokenized)
        {
            if (*tokenized=='<')
            {
                inRedirect(strtok(NULL, " "));
            }
            else if (*tokenized =='>')
            {
                outRedirect(strtok(NULL, " "));
            }
            else if (*tokenized=='|')
            {
                // piping (breakdown,strtok(NULL, " "));
                // free(breakdown);
                // free(tokenlist);
                // return;
                char *first = strtok(linebuffer, "|");
                char *second = strtok(NULL, "");
                piping(first, second); 
            }
            else if (*tokenized=='*')
            {
                //wildcard function
                //wildcards(breakdown);
                free(breakdown);
                free(tokenlist);
                return;
            }
            else
            {

                args[parse]= tokenized;
                //fprintf (stderr, "%s\n", tokenized);
                parse++;
                //checkbarenames(args);
                
            }
        tokenized=strtok(NULL, " ");
        args[parse]=NULL;
        if (access(filepath1, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
            execv(filepath1, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                
                }
                else
                {
            
                    waiting=0;
                }
            }
        }
        //checking second bin
        if (access(filepath2, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
            execv(filepath2, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                
                }
                else
                {
            
                    waiting=0;
                }
            }
        }

        //checking third bin
        if (access(filepath3, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
            execv(filepath3, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                
                }
                else
                {
            
                    waiting=0;
                }
            }
        }
        //checking four bin
        if (access(filepath4, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
            execv(filepath4, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                
                }
                else
                {
            
                    waiting=0;
                }
            }        
        }
        //checking fifth bin
        if (access(filepath5, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
            execv(filepath5, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                
                }
                else
                {
            
                    waiting=0;
                }
            }        
        }
        //checking sixth bin
        if (access(filepath6, F_OK) != -1)
        {
            pid_t pid;
            pid= fork();
            if (pid<0)
            {
                perror("fork");
            }
            else if (pid==0)
            {
                execv(filepath6, args);
            }
            else
            {
                if (waiting)
                {
                    waitpid(pid, NULL, 0);
                }
                else
                {
                    waiting=0;
                }
            }       
         }

    }
    free(breakdown);

    free(linebuffer);
    //args[parse]=NULL;
    return;
}

////////////APPEND FUNCTION/////////////////////
//append fills in the line buffer
void append(char*buf, int len)
{
    int newpos=linepos+len;
    
    if(newpos>linesize)
    {
        linesize*=2;
        assert(linesize>=newpos);
        linebuffer=realloc(linebuffer, linesize);
        if(linebuffer==NULL)
        {
            perror("line buffer");
            failedcommand=1;
        }
    }
    memcpy(linebuffer+linepos+'\0', buf, len);
    linepos=newpos;


}


//////////////// MAIN FUNCTION///////////////////////////////
int main (int argc, char **argv)
{
    int bytes;
    char buffer[BUFFSIZE];
    //if there is an input after this means its opening the file 
    //(batch mode)
    if (argc>1)
    {
        //opening the file in read only
        filno = open(argv[1],O_RDONLY);
        //if file does not exist or we do not have permission
        //return error
        if (filno==-1)
        {
            perror(argv[1]);
            //exit(EXIT_FAILURE);
        }
    }
    else
    {
        filno=0;
    }
    //storage for current line
    linebuffer = malloc(BUFFSIZE);
    linesize = BUFFSIZE;
    linepos = 0;
    int lstart;
    int pos;
         //for interavtive mode we need the >sh in the terminal in indicate
        //its ready for an input and is in interactive mode
        //implement later using global var !>sh after a failed command
        if(argc==1)
        {
            interative=1;
            write(STDOUT_FILENO,"mysh> ", 6);
        }
        
    while ((bytes= read(filno,buffer,BUFFSIZE))>0) {
        lstart=0;
        for(pos=0; pos<bytes;++pos) {
            if(buffer[pos]=='\n') {
                int thisLen = pos - lstart + 1;
                append(buffer + lstart, thisLen);
                get_token();
                linepos = 0;
                lstart = pos + 1;
            }
        }
        if(lstart<bytes) {
            int thisLen=pos-lstart;
            append(buffer+lstart, thisLen);
        }
        if (linepos>0) {
            append("\n",1);
            get_token();
        }
        if(isatty(filno)) {
           if (pwdwait==1) {
                pwdwait=0;
            } else {
                 if (interative==1) {
                    if(failedcommand == 1) {
                        write(STDIN_FILENO,"!mysh> ",7);
                        failedcommand = 0;
                    }else {
                        write(STDIN_FILENO,"mysh> ",6);
                    }
                }
            }
        }
    }
    free(linebuffer);
    close(filno);
}

