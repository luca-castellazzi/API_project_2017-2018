#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TR "tr"   /* Identifier for the start of the list of transitions */
#define ACC "acc" /* Identifier for the start of the list of final states */
#define MAX "max" /* Identifier for the max number of moves */
#define RUN "run" /* Identifier for the start of the list of input strings */

#define CONV_TERM 48      /* Conversion term to convert characters that can appear on the input tape (with 48 all numbers, capital letters and lower case letters are covered) */
#define C 75              /* Length of the array of characters that can be read as input (75 because '0'=48 and 'z'=122 in ASCII) */
#define MAX_KEYWORD_LEN 3 /* Max length of the keywords for input separation (tr, acc, max, run) */
#define BLANK '_'         /* "Blank" codification given by the specifications */

/* Structure to save transitions (states of the Turing Machine) */
typedef struct state_s{
   char whatToWrite;
   char howToMove;
   int whereToGo;
   struct state_s *next;
} adj;
/****************************************************************/

/* Structure for the queueEl *******************************/
typedef struct queueEl_s{
   char whatToWrite;
   char howToMove;
   int whereToGo;
   long unsigned int count;
   int rightLen; /* Length of the right tape */
   char *rightTape;
   int leftLen; /* Length of the (optional) left tape */
   char *leftTape;
   int pos;
   struct queueEl_s *next;
} queueEl;
/***********************************************************/

/* Global variables for the queue */
queueEl *front = NULL;
queueEl *rear = NULL;
/**********************************/

/* Function prototipes *******************************************************************/
void addTrInfo(adj *transaction, char charToWrite, char moveToPerform, int nextState);
void enqueue(queueEl *new);
void deallocateQueue();
queueEl * dequeue();
/*****************************************************************************************/

/* main */
int main(int argc, char *argv[]){

   /* Variables useful to get the input *********************************************************************/
   char keyword[MAX_KEYWORD_LEN + 1];
   char readChar, charToWrite, moveToPerform;
   int currState, nextState, readCharIdx;
   char *dynamicTmpStr = NULL;
   int len, finalStateIdx, finalState, *finalStates = NULL, flag;
   long unsigned int maxVal;
   adj ***graph = NULL;     /* Structure to represent the graph of states of the Turing Machine */
   adj **readChars = NULL;  /* Structure to store the transactions relative to a single read character */
   adj *transaction = NULL; /* Actual structure for the transaction */
   /*********************************************************************************************************/


   /* Variables useful to the functioning of the Turing Machine **************************/
   char *blank = NULL, out, nextChar;
   adj *toEnqueue = NULL;
   int accept, inputLen, additionalBlanks;
   queueEl *currQueueEl = NULL, *new = NULL;
   /**************************************************************************************/


/* Read tr section *****************************************************************************************************/
   scanf("%s", keyword);
   if(!strcmp(keyword, TR)) {
        flag = 1; /* flag = 1 only for the first read transaction */
        scanf("%s", keyword);
        while(strcmp(keyword, ACC)) {
            scanf(" %c %c %c %d", &readChar, &charToWrite, &moveToPerform, &nextState);
            currState = atoi(keyword);
            readCharIdx = (int)readChar - CONV_TERM;

            /* Create the graph */
            if(flag == 1) {
                readChars = (adj **)calloc(1, C*sizeof(adj *));
                transaction = (adj *)calloc(1, sizeof(adj));
                addTrInfo(transaction, charToWrite, moveToPerform, nextState);
                transaction->next = readChars[readCharIdx];
                readChars[readCharIdx] = transaction;
                graph = (adj ***)calloc(1, sizeof(adj **));
                graph[currState] = readChars;
                flag = 0;
                len = 1 + currState;
            }
            else{
                if(currState < len){
                    if(graph[currState] == NULL){
                        readChars = (adj **)calloc(1, C*sizeof(adj *));
                        transaction = (adj *)calloc(1, sizeof(adj));
                        addTrInfo(transaction, charToWrite, moveToPerform, nextState);
                        transaction->next = readChars[readCharIdx];
                        readChars[readCharIdx] = transaction;
                        graph[currState] = readChars;
                    }
                    else{
                        transaction = (adj *)calloc(1, sizeof(adj));
                        addTrInfo(transaction, charToWrite, moveToPerform, nextState);
                        transaction->next = graph[currState][readCharIdx];
                        graph[currState][readCharIdx] = transaction;
                    }
                }
                else{ /* Realloc graph */
                    readChars = (adj **)calloc(1, C*sizeof(adj *));
                    transaction = (adj *)calloc(1, sizeof(adj));
                    addTrInfo(transaction, charToWrite, moveToPerform, nextState);
                    transaction->next = readChars[readCharIdx];
                    readChars[readCharIdx] = transaction;
                    graph = (adj ***)realloc(graph, (1 + currState)*sizeof(adj **));
                    for(int i = len; i <= currState; i++) {
                        graph[i] = NULL;
                    }
                    graph[currState] = readChars;
                    len = 1 + currState;
                }
            }
            scanf("%s", keyword);
      }
   }
   else {
       printf("Error in input format: it must start with 'tr'");
   }
/***********************************************************************************************************************/


/* Read acc section *******************************************************************************************************************************************/
    finalStateIdx = 0;
    scanf("%ms", &dynamicTmpStr); /* Let's use a dynamically-allocated string because the number of final states and their single length are unknown */
    while(strcmp(dynamicTmpStr, MAX)){
        finalState = atoi(dynamicTmpStr);
        if(finalStateIdx == 0){
            finalStates = (int *)malloc(sizeof(int));
            *(finalStates) = finalState;
            finalStateIdx++;
        }
        else{
            finalStates = (int *)realloc(finalStates, (finalStateIdx + 1)*sizeof(int));
            finalStates[finalStateIdx] = finalState;
            finalStateIdx++;
        }
        free(dynamicTmpStr);
        scanf("%ms", &dynamicTmpStr);
    }
    free(dynamicTmpStr);
/**************************************************************************************************************************************************************/


/* Read max section *****************/
   scanf("%lu", &maxVal);
/************************************/


/* Read run section and Actual Turing Machine Logic *************************************************************************************************/
   scanf("%s", keyword);
   if(!strcmp(keyword, RUN)){
        while(scanf("%ms", &dynamicTmpStr) != EOF){
            inputLen = strlen(dynamicTmpStr);
            accept = -1;
            /* If I can move from 0 with the first char of the input string... */
            if(graph[0][(int)(*dynamicTmpStr) - CONV_TERM] != NULL){
                toEnqueue = graph[0][(int)(*dynamicTmpStr) - CONV_TERM];
                /* ...then I add to the queueEl all the adjacent states w.r.t. 0 that I can reach with the first char of the input string  */
                for(adj *v = toEnqueue; v != NULL; v = v->next){
                    new=(queueEl *)malloc(sizeof(queueEl)); /* New tmp instance for the queueEl */
                    new->whatToWrite = v->whatToWrite;
                    new->howToMove = v->howToMove;
                    new->whereToGo = v->whereToGo;
                    new->count = 0;
                    new->rightLen = inputLen;
                    new->rightTape = NULL;
                    new->rightTape = (char *)malloc((1 + new->rightLen)*sizeof(char));
                    memcpy(new->rightTape, dynamicTmpStr, 1 + inputLen);
                    new->leftLen = 0;
                    new->leftTape = NULL;
                    new->pos = 0;
                    new->next = NULL;
                    enqueue(new);
                }
                free(dynamicTmpStr);

                /* While there are still elements in the queueEl... */
                while(front != NULL){
                    /* Dequeue */
                    currQueueEl = dequeue();
                    /* Check if the "whereToGo" state of currQueueEl is in the list of final states in order to accept the input string*/
                    for(int i = 0; i < finalStateIdx; i++) {
                        if(finalStates[i] == currQueueEl->whereToGo){
                            accept = 1;
                            break;
                        }
                    }
                    /* If the input string is accepted... */
                    if(accept == 1){
                        /* ...then deallocate currQueueEl */
                        free(currQueueEl->rightTape);
                        currQueueEl->rightTape = NULL;
                        free(currQueueEl->leftTape);
                        currQueueEl->leftTape = NULL;
                        free(currQueueEl);
                        currQueueEl = NULL;
                        /* And deallocate the whole queue */
                        deallocateQueue();
                        break;
                    }
                    /* If, going in "currQueueEl" state, the max number of moves would have been exceeded... */
                    else if(currQueueEl->count > maxVal){
                        /* ...then the result of the computation would be "U" */
                        accept = 2;
                        /* Deallocate currQueueEl */
                        free(currQueueEl->rightTape);
                        free(currQueueEl->leftTape);
                        free(currQueueEl);
                    }
                    else{
                        if(currQueueEl->whereToGo < len){
                            /* If the "whereToGo" state of currQueueEl does exists in the graph... */
                            if(graph[currQueueEl->whereToGo] != NULL){
                                currQueueEl->count++;
                                /* ...then write... */
                                if(currQueueEl->pos >= 0) {
                                    currQueueEl->rightTape[currQueueEl->pos] = currQueueEl->whatToWrite;
                                }
                                else {
                                    currQueueEl->leftTape[(-1)*(currQueueEl->pos)-1] = currQueueEl->whatToWrite;
                                }
                                /* ...and move */
                                /* In case of move to the LEFT... */
                                if(currQueueEl->howToMove=='L'){
                                    currQueueEl->pos--;
                                    if(currQueueEl->pos < 0){
                                        if(currQueueEl->leftTape == NULL){
                                            /* ...create the left tape if needed */
                                            currQueueEl->leftLen = 20;
                                            currQueueEl->leftTape = (char *)malloc(21*sizeof(char));
                                            memset(currQueueEl->leftTape, BLANK, 21);
                                            currQueueEl->leftTape[20] = '\0';
                                        }
                                        nextChar = currQueueEl->leftTape[(-1)*(currQueueEl->pos) - 1];
                                        /* If the left tape is already ended... */
                                        if(nextChar == '\0'){
                                            nextChar=BLANK;
                                            /* ...make it infinite adding space */
                                            additionalBlanks = currQueueEl->rightLen / 2;
                                            blank = (char *)malloc((1 + additionalBlanks)*sizeof(char));
                                            memset(blank, BLANK, 1 + additionalBlanks);
                                            blank[additionalBlanks] = '\0';
                                            currQueueEl->leftLen = currQueueEl->leftLen+additionalBlanks;
                                            currQueueEl->leftTape = (char *)realloc(currQueueEl->leftTape, (1 + currQueueEl->leftLen)*sizeof(char));
                                            memcpy(currQueueEl->leftTape + (currQueueEl->leftLen - additionalBlanks), blank, 1 + additionalBlanks);
                                            free(blank);
                                        }
                                    }
                                    else{
                                        nextChar = currQueueEl->rightTape[currQueueEl->pos];
                                    }
                                }
                                /* In case of move to the RIGHT... */
                                else if(currQueueEl->howToMove == 'R'){
                                    currQueueEl->pos++;
                                    if(currQueueEl->pos >= 0){
                                        /* ...then allocate an additional part of blanks to the right tape, if needed */
                                        if(currQueueEl->rightTape[currQueueEl->pos] == '\0'){
                                            additionalBlanks = 20;
                                            blank=(char *)malloc(21*sizeof(char));
                                            memset(blank, BLANK, 21);
                                            blank[additionalBlanks]='\0';
                                            currQueueEl->rightLen=currQueueEl->rightLen+20;
                                            /* And the right tape is increased */
                                            currQueueEl->rightTape=(char *)realloc(currQueueEl->rightTape, (1 + currQueueEl->rightLen)*sizeof(char));
                                            memcpy(currQueueEl->rightTape + (currQueueEl->rightLen - 20), blank, 21);
                                            free(blank);
                                        }
                                        nextChar = currQueueEl->rightTape[currQueueEl->pos];
                                    }
                                    else {
                                        nextChar = currQueueEl->leftTape[(-1)*(currQueueEl->pos) - 1];
                                    }
                                }
                                /* Final case in which the move is to STAY in the current position */
                                else if(currQueueEl->howToMove == 'S'){
                                    if(currQueueEl->pos >= 0) {
                                        nextChar = currQueueEl->rightTape[currQueueEl->pos];
                                    }
                                    else {
                                        nextChar = currQueueEl->leftTape[(-1)*(currQueueEl->pos) - 1];
                                    }
                                }

                                if(graph[currQueueEl->whereToGo][(int)nextChar - CONV_TERM] != NULL){
                                    toEnqueue = graph[currQueueEl->whereToGo][(int)nextChar - CONV_TERM];
                                    /* I can add to the queueEl all the adjacent states (and all the ones that are actually reachable) w.r.t. the last examined state */
                                    adj *tmp = NULL; /* Temp variable for the following for loop (it is needed to have the last element after the for loop) */
                                    for(tmp = toEnqueue; tmp->next != NULL; tmp = tmp->next){
                                        new = (queueEl *)malloc(sizeof(queueEl));
                                        new->whatToWrite = tmp->whatToWrite;
                                        new->howToMove = tmp->howToMove;
                                        new->whereToGo = tmp->whereToGo;
                                        new->count = currQueueEl->count;
                                        new->rightLen = currQueueEl->rightLen;
                                        new->rightTape = NULL;
                                        new->rightTape = (char *)malloc((1 + new->rightLen)*sizeof(char));
                                        memcpy(new->rightTape, currQueueEl->rightTape, 1 + new->rightLen);
                                        new->leftTape = NULL; //Inizializzazione
                                        if(currQueueEl->leftTape != NULL){
                                            new->leftLen = currQueueEl->leftLen;
                                            new->leftTape = (char *)malloc((1 + new->leftLen)*sizeof(char));
                                            memcpy(new->leftTape, currQueueEl->leftTape, 1 + new->leftLen);
                                        }
                                        else{
                                            currQueueEl->leftLen = 0;
                                            currQueueEl->leftTape = NULL;
                                        }
                                        new->pos = currQueueEl->pos;
                                        new->next = NULL;
                                        enqueue(new);
                                    }
                                    /* Re-use the old currQueueEl allocation */
                                    currQueueEl->whatToWrite = tmp->whatToWrite;
                                    currQueueEl->howToMove = tmp->howToMove;
                                    currQueueEl->whereToGo = tmp->whereToGo;
                                    currQueueEl->next = NULL;
                                    enqueue(currQueueEl);
                                } /* Close the if about the existence of graph[currQueueEl->whereToGo][(int)nextChar - CONV_TERM] */
                                else {
                                    /* This branch did NOT accept the input string, but if another one gave "U", than U "wins" */
                                    if(accept != 2) {
                                        accept = 0;
                                    }
                                    //Deallocate currQueueEl
                                    free(currQueueEl->rightTape);
                                    free(currQueueEl->leftTape);
                                    free(currQueueEl);
                                }
                            } /* Close the if about the existence of graph[currQueueEl->whereToGo] */
                            else {
                                if(accept!=2) {
                                    accept=0;
                                }
                                //Deallocate currQueueEl
                                free(currQueueEl->rightTape);
                                free(currQueueEl->leftTape);
                                free(currQueueEl);
                            }
                        } /* Close the if about the particular cases (states with len > graph len) */
                        else {
                            if(accept != 2) {
                                accept = 0;
                            }
                            //Deallocate currQueueEl
                            free(currQueueEl->rightTape);
                            free(currQueueEl->leftTape);
                            free(currQueueEl);
                        }
                    } /* Close the if for the move (non-final state and currQueueEl->count <= max) */
                } /* Close the while to know if the queueEl is still non-empty */
            } /* Close the if about the movement from 0 */
            else {
                accept = 0;
            }

            switch(accept)
            {
            case 0:
                out = '0';
                break;
            case 1:
                out = '1';
                break;
            default:
                out = 'U';
                break;
            }   

            printf("%c\n", out);
        }
   }
/****************************************************************************************************************************************************/


/* Free the last allocated elements ******************/
   for(int i = 0; i < C; i++) {
       free(readChars[i]);
   }

   for(int i = 0; i < len; i++) {
       free(graph[i]);
   }

   free(finalStates);
/*****************************************************/


   return 0;
}


/* Functions ********************************************************************************/
void addTrInfo(adj *transaction, char charToWrite, char moveToPerform, int nextState) {
    transaction->whatToWrite = charToWrite;
    transaction->howToMove = moveToPerform;
    transaction->whereToGo = nextState;
    return;
}

void enqueue(queueEl *new) {
    if(front == NULL && rear == NULL){
        front = new;
        rear = new;
    }
    else{
        rear->next = new;
        rear = new;
    }

    return;
}

void deallocateQueue() {
    queueEl *p = NULL;
    while(front != rear){
        p = front;
        front = front->next;
        free(p->rightTape);
        free(p->leftTape);
        free(p);
    }
    if(front != NULL){
        free(front->rightTape);
        free(front->leftTape);
        free(front);
        front = NULL;
        rear = NULL;
    }

    return;
}

queueEl *dequeue(){
   queueEl *temp = front;

   if(front == rear) {
		front = NULL;
        rear = NULL;
   }
   else {
   	    front = front->next;
   }

   return temp;
}
/********************************************************************************************/