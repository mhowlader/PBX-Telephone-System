/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include "csapp.h"

typedef struct tu {
    int tu_fd; //the file descriptor of the TU
    int ext; //extension number of the TU
    int ref; //reference number of the TU
    TU_STATE cur_state;
    struct tu *peer;
    sem_t tu_mutex;
} TU;

//access tu has to have been locked beforehand
//if state is connected then access to the peer tu also locked beforehand.
int tu_send_current_state(TU *tu) {
    switch (tu->cur_state) {
        case TU_ON_HOOK:
            if (dprintf(tu->tu_fd,"%s %d\r\n",tu_state_names[tu->cur_state],tu->ext)<0) {
                return -1;
            }
            break;
        case TU_CONNECTED:
            if (dprintf(tu->tu_fd,"%s %d\r\n",tu_state_names[tu->cur_state],tu->peer->ext)<0) {
                return -1;
            }
            break;
        default:
            if (dprintf(tu->tu_fd,"%s\r\n",tu_state_names[tu->cur_state])<0) {
                return -1;
            }
            break;
    }
    return 0;
}

//associate tu_mutexes with extensions
//sem_t tu_mutex[PBX_MAX_EXTENSIONS];
/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 * //modifies TU
 */
#if 1
TU *tu_init(int fd) {
    TU *newTU = calloc(1,sizeof(TU));
    newTU->tu_fd=fd;
    newTU->cur_state = TU_ON_HOOK; 
    Sem_init(&(newTU->tu_mutex),0,1);
    return newTU;
}
#endif

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 * //modifies TU
 */
#if 1
void tu_ref(TU *tu, char *reason) {
    //P(&(tu->tu_mutex));
    tu->ref++;
    //V(&(tu->tu_mutex));
}
#endif

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 * //modifies TU
 */
#if 1
void tu_unref(TU *tu, char *reason) {
    //P(&(tu->tu_mutex));
    tu->ref--;
    //V(&(tu->tu_mutex));
    if (tu->ref==0) {
        free(tu);
        //tu=NULL;
    }
}
#endif

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 * //read TU
 */
#if 1
int tu_fileno(TU *tu) {
    int fd = -1;
    P(&(tu->tu_mutex));
    fd=tu->tu_fd;
    V(&(tu->tu_mutex));
    return fd;
}
#endif

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 * 
 * @param tu
 * @return the extension number, if any, otherwise -1.
 * read TU
 */
#if 1
int tu_extension(TU *tu) {
    int ext = -1;
    P(&(tu->tu_mutex));
    ext = tu->ext;
    V(&(tu->tu_mutex));
    return ext;
}
#endif

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 * modify TU
 */
#if 1
int tu_set_extension(TU *tu, int ext) {
    int ret=0;
    P(&(tu->tu_mutex));
    tu->ext=ext;
    if (dprintf(tu->tu_fd,"%s %d\r\n",tu_state_names[tu->cur_state],tu->ext)<0) {
        ret=-1;
    }
    V(&(tu->tu_mutex));
    return ret;
}
#endif

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 * modify two TUs
 */
#if 1
int tu_dial(TU *tu, TU *target) {
    int ret = 0;
    P(&(tu->tu_mutex));
    if (tu->cur_state==TU_DIAL_TONE) {
        if (tu==target) {
            tu->cur_state = TU_BUSY_SIGNAL;
        }
        else if (target==NULL) {
            tu->cur_state=TU_ERROR;
        } 
        else {
            P(&(target->tu_mutex));
            if (target->peer!=NULL || target->cur_state!=TU_ON_HOOK) {
                tu->cur_state = TU_BUSY_SIGNAL;
            }
            else {
                tu->peer=target;
                target->peer=tu;
                tu_ref(tu,"Dialed a valid TU");
                tu_ref(target,"Received valid call from TU");
                tu->cur_state=TU_RING_BACK;
                target->cur_state=TU_RINGING;
                if (tu_send_current_state(target)==-1) {
                    ret = -1;
                }
            }
            V(&(target->tu_mutex));
        }
    }
    if (tu_send_current_state(tu)==-1) {
        ret = -1;
    }
    V(&(tu->tu_mutex));
    return ret;

}
#endif

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state.
 * can modify two TUs
 */
#if 1
int tu_pickup(TU *tu) {
    int ret = 0;
    P(&(tu->tu_mutex));
    if (tu->cur_state==TU_ON_HOOK) {
        tu->cur_state=TU_DIAL_TONE;
    }
    else if (tu->cur_state==TU_RINGING) {
        if(tu->peer!=NULL) {
            P(&(tu->peer->tu_mutex)); //lock the peer
            tu->peer->cur_state=TU_CONNECTED;
            tu->cur_state=TU_CONNECTED;
            if (tu_send_current_state(tu->peer)==-1) {
                ret=-1;
            }
            V(&(tu->peer->tu_mutex));
        }
        else {
            ret = -1;
        }
    }
    if (tu_send_current_state(tu)==-1) {
        ret = -1;
    }
    V(&(tu->tu_mutex));
    return ret;
}
#endif

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 * modify two TUs
 */
#if 1
int tu_hangup(TU *tu) {
    int ret = 0;
    P(&(tu->tu_mutex));
    if (tu->cur_state==TU_CONNECTED || tu->cur_state == TU_RINGING) {
        tu->cur_state=TU_ON_HOOK;
        if (tu->peer!=NULL) {
            P(&(tu->peer->tu_mutex));
            tu->peer->cur_state=TU_DIAL_TONE;
            tu_unref(tu->peer,"Hangup");
            tu_unref(tu,"Hangup");
            tu->peer->peer=NULL;

            if (tu_send_current_state(tu->peer)==-1) {
                ret=-1;
            }
            V(&(tu->peer->tu_mutex));
        }
        else {
            ret = -1;
        }

    }
    else if (tu->cur_state==TU_RING_BACK) {
        tu->cur_state=TU_ON_HOOK;
        if(tu->peer!=NULL) {
            P(&(tu->peer->tu_mutex)); //lock the peer
            tu->peer->cur_state=TU_ON_HOOK;
            tu_unref(tu->peer,"Hangup");
            tu_unref(tu,"Hangup");
            tu->peer->peer=NULL;
            if (tu_send_current_state(tu->peer)==-1) {
                ret=-1;
            }
            V(&(tu->peer->tu_mutex));
        }
        else {
            ret= -1;
        }
    }
    else if(tu->cur_state==TU_DIAL_TONE || tu->cur_state==TU_BUSY_SIGNAL || tu->cur_state==TU_ERROR) {
        tu->cur_state=TU_ON_HOOK;
    }
    tu->peer=NULL;

    if (tu_send_current_state(tu)==-1) {
        ret = -1;
    }
    V(&(tu->tu_mutex));
    return ret;
}
#endif

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 * reads two TUs
 * 

 */
#if 1
int tu_chat(TU *tu, char *msg) {
    int ret = 0;
    P(&(tu->tu_mutex));
    if (tu->cur_state==TU_CONNECTED) {
        if (tu->peer!=NULL) {
            if (dprintf(tu_fileno(tu->peer),"CHAT %s\r\n",msg)<0) {
                ret = -1;
            }
        }
        else {
            ret = -1;
        }
    }
    else {
        ret = -1;
    }
    tu_send_current_state(tu);
    V(&(tu->tu_mutex));
    return ret;
}
#endif
