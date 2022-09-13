#include <stdlib.h>
#include <unistd.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

volatile sig_atomic_t sighup_called = 0;

static void terminate(int status);

void sighup_handler(int sig) {
    //don't call termiante in handler
    sighup_called = 1;
}

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.


    int c = getopt(argc,argv,"p:");
    if (c!='p') {
        fprintf(stderr,"Usage: bin/pbx -p <port>\n");
        exit(EXIT_SUCCESS);
    }
    
    
    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.


    struct sigaction action, old_action;

    action.sa_handler = sighup_handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = 0; /* Restart syscalls if possible */
    if (sigaction(SIGHUP, &action, &old_action) < 0)
        unix_error("Signal error");
    
    //Signal(SIGPIPE,SIG_IGN);//

    struct sigaction ignoreaction;

    ignoreaction.sa_handler = SIG_IGN;  
    sigemptyset(&ignoreaction.sa_mask); /* Block sigs of type being handled */
    ignoreaction.sa_flags = 0;
    if (sigaction(SIGPIPE, &ignoreaction, NULL) < 0)
	    unix_error("Signal error");

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 


    listenfd = Open_listenfd(optarg);
    int optval;
    socklen_t optlen = sizeof(optval);
    /* Check the status for the keepalive option */
    if (getsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0)
    {
        perror("getsockopt()");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    //printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

    /* Set the option active */
    optval = 1;
    optlen = sizeof(optval);
    if (setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
    {
        perror("setsockopt()");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    //printf("SO_KEEPALIVE set on socket\n");

    while(!sighup_called) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));                              // line:conc:echoservert:beginmalloc
        *connfdp = accept(listenfd, (SA *)&clientaddr, &clientlen); // line:conc:echoservert:endmalloc
        if (*connfdp < 0) {
            if (errno==EINTR) {
                free(connfdp);
            }
            break;
        }
        Pthread_create(&tid, NULL, pbx_client_service, connfdp); 
    }
    

    //check if error and not eintr caused by interrupted signal
    if (errno && errno!= EINTR) {        
        terminate(EXIT_FAILURE);
    }
    terminate(EXIT_SUCCESS); 
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}
