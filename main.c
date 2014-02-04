/*
 * main.c: A simple shell final
 * Authors: Muhammed Anwar & Shaham Farooq
 *            998950470     998964633
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

//Parameters for program
#define BUF_SIZE 10753
#define PROMPT "ece353sh$"

extern int errno;

typedef struct _path
{
    char **directories;
    int size;
}path;

typedef struct _listNode
{
    char* input;
    struct _listNode* next;
}listNode;

typedef struct _linkedList
{
    listNode* head;
    listNode* tail;
    int size;
}linkedList;

/**Input Functions**/
char* getinput(char*,int);
int parseinput(const char*);

/**Execution Function**/
void executeCommand(char *, char**,path*);

/**Dynamic Array Method Prototypes**/
void addDirectories(path*, char*);
void removeDirectories(path*, char*);
void printDirectories(path*);
void destroyAllDirectories(path*);

/**Linked List Method Prototypes**/
void addList(char* data, linkedList* history);
void printList(linkedList* history);
listNode* getItem(int index, linkedList* history);
void deleteList(listNode* list);


int main(int argc, char *argv[]) {
	char buf[BUF_SIZE];               //Input from stdin
	char buf2[BUF_SIZE];              //Copy of buf
	char *args[22];                   //Arguments to the shell
	char *temp;                       //General temporary string
	char *command;                    //Command to the shell
	int i;                            //General Purpose Counter (gpc)
    int invalid;                      //Invalid command flag

	//Initialize the Path structure
	path *p = malloc(sizeof(path));
    p->directories=NULL;
    p->size =0;

    //Initialize the History structure
    linkedList* history = malloc(sizeof(linkedList));
    history->size =0;
    history->head = NULL;
    history->tail = NULL;

	do {
        invalid = 0;
        getinput(buf, sizeof(buf));    //Get the standard input and store it in buf

        /**Tokenize input into command and arguments**/
        strcpy(buf2, buf);             //Copy buf to buf2, as buf will be destroyed by strtok
        temp = strtok (buf," \n");     //Delimit by \n to remove the last \n at the end of the command
        command=temp;                  //First token is the command
        args[0] = command;
        i=1;
        while(i<22)                    //Get the rest of the tokens
        {

            temp=strtok (NULL," \n");
            args[i]=temp;               //The rest of the tokens are the arguments of the command
            i++;
        }
        if(command!=NULL)               //If we have some commands
        {
            if(strcmp(command,"exit")==0 || strcmp(command,"exit\n")==0)invalid =1; //If command is exit, dont execute it just quit

            if(command[0]=='!')
            {
                int n = atoi(command+1);
                if(n<=0 || n>100) {printf("error: Command Not Found\n"); invalid = 1;}
                else{
                    listNode* tempNode = getItem(n,history);
                    strcpy(buf,tempNode->input);
                    strcpy(buf2,tempNode->input);

                    temp = strtok (buf," \n");     //Delimit by \n to remove the last \n at the end of the command
                    command=temp;                  //First token is the command
                    args[0] = command;
                    i=1;
                    while(i<22)                    //Get the rest of the tokens
                    {

                        temp=strtok (NULL," \n");
                        args[i]=temp;               //The rest of the tokens are the arguments of the command
                        i++;
                    }
                }
            }
            addList(buf2,history);
            if(strcmp(command,"history")==0 || strcmp(command,"history\n")==0) printList(history);
            else if(strcmp(command,"cd")==0 || strcmp(command,"cd\n")==0){
                        if (chdir(args[1])==-1 && args[1]!=NULL) printf("error: %s\n",strerror(errno));
            }
            else if(strcmp(command, "path")==0 || strcmp(command,"path\n")==0)
            {
                if (args[1] == NULL || (strcmp(args[1],"+")!=0 && strcmp(args[1],"-")!=0)) printDirectories(p);
                else if (strcmp(args[1],"+")==0 && args[2]!=NULL) addDirectories(p,args[2]);
                else if (strcmp(args[1],"-")==0 && args[2]!=NULL) removeDirectories(p,args[2]);
                //else printf("error: command arguments not valid\n");
            }
            else if(invalid ==0)
            {
                executeCommand(command,args,p); //Call execute to run the program
            }

        }
    } while (parseinput(buf2) == 1);

    destroyAllDirectories(p);           //Clean up all remaining elements in the array
    free(p);                            //Free the path structure
    deleteList(history->head);          //Delete the whole history list
    free(history);                      //Delete the history
	return 0;
}

/** Execute Command**/
void executeCommand(char *command, char* args[], path* p)
{
    pid_t child_pid;                    //The PID for the child process
    child_pid = fork();                 //Fork the current process, to create a copy
    int i;
    

    if(child_pid==0)                    //If we are in the child process
    {
		//if(strlen(command)>=2){
			if(command[0]=='.' && command[1]=='/')
				execv(command,args);
		//}
      else{
        for(i=0;i<p->size; i++)
        {
            char temp[1024];
            strcpy(temp, p->directories[i]);
            strcat(temp,"/");
            strcat(temp,command);
            execv(temp,args);
        }
      }
	// execv(command,args);            //call execv to execute program
        printf("error: Command Not Found\n");  //If program comes here, couldn't find executable
        exit(0);                        //Exit out the the rogue child process
    }
    else                                //If we are in the parent
    {
        int status;
        (void)waitpid(child_pid, &status, 0);   //wait for the child process to exit
    }
}
/** END - Execute Command**/

/**Input Function definitions**/
char *getinput(char *buf, int len) {
	printf("%s ", PROMPT);
	return fgets(buf, len, stdin);
}

int parseinput(const char *buf) {
	if (strcmp(buf, "exit\n") == 0)
		return 0;
	return 1;
}
/**END - Input Function definitions**/


/**Dynamic Array Function definitions**/
/*addDirectories: p - path to add the directory too
                  directory - the string directory to add*/
void addDirectories(path* p, char* directory)
{
    if(p->directories==NULL)                            //If we have an empty array
    {
        p->directories = malloc(sizeof(char*));         //Allocate a single new element
        p->directories[0]= malloc(strlen(directory)+1); //In the new element, allocate enough space for the directory
    }
    else                                                //If we already have some values in the array
    {
        char** temp;                                    //Reallocate the space for the directory array
        temp = realloc(p->directories,((p->size)+1)*sizeof(char*)); //Realloc resizes the array to +1 to the current size
        p->directories = temp;
        p->directories[p->size]= malloc(strlen(directory)+1);       //In the expanded array, allocate enough space for the directory
    }
    strcpy(p->directories[p->size],directory);          //copy the directory to the array
    (p->size)++;
}
/*removeDirectories: p - path to add the directory too
                     directory - the string directory to remove*/
void removeDirectories(path* p, char* directory)
{
    if(p==NULL || directory ==NULL) return; //If the path is valid

    int i=0,j;
    while(i<p->size && strcmp(p->directories[i],directory)!=0) i++; //Find the directory to be removed
    if (i==p->size) return;                 //Couldn't find the element return

    free(p->directories[i]);                //free the memory of the directory being deleted
    p->directories[i] =NULL;                //make the pointer null
    for(j=i; j<(p->size-1); j++)            //Shift the elements of the array over the deleted directory
    {
        p->directories[j] = p->directories[j+1];
    }

    p->size--;                              //Update the size
}


void printDirectories(path* p)
{
    if (p!=NULL) //Loop through the array and just print each directory element
    {
        int i;
        for(i=0; i<p->size; i++)
        {	  
	  if(i==p->size-1) printf("%s ", p->directories[i]);
	  else printf("%s: ",p->directories[i]);
        }
    }
    printf("\n");
}
void destroyAllDirectories(path* p)
{
    int i;  //Loop through the array and free the individual directories
    for(i=0; i<(p->size); i++)
        free(p->directories[i]);
    free(p->directories);   //Free the directory array
    p->size = 0;
    return;
}
/**END - Dynamic Array Function definitions**/

/**Linked List Methods**/
//printList - history: The linked list to print
void printList(linkedList* history)
{
    if(history->head == NULL) return;
    listNode* temp = history->head;
    int i=0;
    while(temp!=history->tail){                     //Loop till the very last element
        printf("[%d] %s",++i,temp->input);                 //Print the data
        temp = temp->next;                          //Go to the next element
    }
    printf("[%d] %s",++i,temp->input);                     //Print the last element
}

//addList - data: data to be added
//          history: the list to be added too
void addList(char* data, linkedList* history)
{
    listNode* temp = malloc(sizeof(listNode));      //Create a new node
    temp->input = malloc(strlen(data)+1);           //Copy the data to the node
    strcpy(temp->input,data);
    temp->next = NULL;                              //Set next to NULL
    if(history->head ==NULL)                        //If the list is empty
        history->head = history->tail = temp;       //Head and Tail point to the same new node
    else{                                           //If the list has previous elements
        history->tail->next = temp;                 //The last element now points to the new element
        history->tail = temp;                       //The last element now becomes the new element
    }
    if(history ->size <100) history->size ++;       //If we are not at capacity, increase the size
    else                                            //If we are at capacity, remove the head
    {
        listNode* temp = history->head;             //Move head to a temporary variable
        history->head = temp->next;                 //The new head, is now the second element
        free(temp->input);                          //Free the previous head
        free(temp);
    }
}

//getItem - returns: the node at the index requested
//          index: the index of the item to retrieve
//          history: the linked list to search
listNode* getItem(int index, linkedList* history)
{
    listNode *temp = history->head;                 //Start from the first node
    while(index>1){ temp = temp->next; index --;}   //Keep going to the next node until our index becomes 1
    return temp;                                    //Thats the element we are searching for.

}

//deleteList - history: the first node of the linked list
void deleteList(listNode* history)
{
    //Recursively delete the whole list
    if(history==NULL) return;                       //base case
    deleteList(history->next);                      //recurse until the end of the list
    free(history->input);                           //free the data and the node
    free(history);
}
/**END - Linked List Methods**/
