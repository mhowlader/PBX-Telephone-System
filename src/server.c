/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h"


//parse and executre a message received form a TU 
//return 0 if successful -1 if error
int execute_client_message(TU *curTU,char *buf, int messageSize) {
    if (strcmp(buf,"pickup\r\n")==0) {
        //printf("picksss\n");
        return tu_pickup(curTU);
            //error
    }
    else if (strcmp(buf,"hangup\r\n")==0) {
        return tu_hangup(curTU);
    }
    else {
        char d_command[6];
        memcpy(d_command,buf,5);
        d_command[5] = '\0';
        //printf("%s",d_command);
        if (strcmp(d_command,"dial ")==0) {
            int ext=0;
            char* num_start = buf + 5; //start of where the numebr should be
            while (*num_start!='\r') {
                
                if (isdigit(*num_start)) {
                    ext = (ext*10) + (*num_start-'0');
                    num_start++;
                }
                else {
                    return -1;
                }
            }
            return pbx_dial(pbx,curTU,ext);
        }
        else if (strcmp(d_command,"chat ")==0) {
            buf[messageSize-1]='\0';
            char* message = buf + 5;
            return tu_chat(curTU,message);
        }
    }
    return -1;
}

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 1
void *pbx_client_service(void *arg) {
    int connfd = *((int *)arg);
    Pthread_detach(pthread_self());
    free(arg);
    TU *newTU = tu_init(connfd);
    if (pbx_register(pbx,newTU,connfd)<0) {

    }   

    rio_t rio;
    int n;
    rio_readinitb(&rio,connfd);
    char* buf = malloc(MAXLINE);
    //char buf[MAXLINE];
    while(1) {
        // char command[10]; //command
        // int com_ind = 0;
        // char onechar;
        //rio_readinitb(&rio,connfd);
        // while (rio_readnb(&rio,&onechar,1)!=0) {
        //     if (onechar==' ') {
        //         break;
        //     }
        //     command[com_ind] = onechar;
        //     com_ind++;
        // }
        
        n=rio_readlineb(&rio,buf,MAXLINE);
        if (n==0) {
            // if (tu_hangup(newTU)==-1) {
                
            // }
            break;
        }
        if (n==-1) {
            //error
            break;
        }
        execute_client_message(newTU,buf,n);
    }
    free(buf);
    pbx_unregister(pbx,newTU);
    Close(connfd);
    return NULL;
}
#endif
