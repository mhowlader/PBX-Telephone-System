/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include "csapp.h"

sem_t pbx_mutex;

volatile long thread_cnt = 0;
sem_t thread_cnt_mutex;

sem_t shutdown_mutex;
typedef struct pbx {
    TU* tu_list[PBX_MAX_EXTENSIONS];
    int tu_count;
} PBX;


/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
#if 1
PBX *pbx_init() {
    Sem_init(&pbx_mutex,0,1);
    Sem_init(&shutdown_mutex,0,1);
    Sem_init(&thread_cnt_mutex,0,1);
    return calloc(1,sizeof(PBX));
}
#endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
#if 1
void pbx_shutdown(PBX *pbx) {
    P(&pbx_mutex);
    for (int i=0;i<PBX_MAX_EXTENSIONS;i++) {
        if (pbx->tu_count==0) {
            break;
        }
        if (pbx->tu_list[i]!=NULL) {
            shutdown(tu_fileno(pbx->tu_list[i]),SHUT_RDWR);
            pbx->tu_count--;
            //while (pbx->tu_list!= NULL);
            //pbx_unregister(pbx,pbx->tu_list[i]);
        }
    }
    
    V(&pbx_mutex);
    //while(pbx->tu_count!=0);
    P(&shutdown_mutex);
    free(pbx);
    pbx=NULL;
    V(&shutdown_mutex);

}
#endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
#if 1
int pbx_register(PBX *pbx, TU *tu, int ext) {
    int ret = 0;
    P(&pbx_mutex);
    if (pbx->tu_list[ext]==NULL) {
        pbx->tu_list[ext]=tu; 
    }
    else{
        ret = -1;
        //no reason why the tu_list at that index should not be null but just in case
    }
    pbx->tu_count++;
    V(&pbx_mutex);
    tu_ref(tu,"Registering TU with PBX");
    tu_set_extension(tu,ext);

    P(&thread_cnt_mutex);
    thread_cnt++;
    if (thread_cnt==1) {
        P(&shutdown_mutex);
    }
    V(&thread_cnt_mutex);
    return ret;
}
#endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
    * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 * 
 * //DOESN"T LOCK BEFOREHAND< PBX NEEDDS TO BE LOCKED BY CALLER
 */
#if 1
int pbx_unregister(PBX *pbx, TU *tu) {
    int ret = 0;
    P(&pbx_mutex);
    pbx->tu_list[tu_extension(tu)] = NULL;
    //pbx->tu_count--;
    V(&pbx_mutex);
    if (tu_hangup(tu) == -1) {
        ret = -1;
    }
    
    tu_unref(tu,"Unregistering TU from PBX");
    P(&thread_cnt_mutex);
    thread_cnt--;
    if (thread_cnt==0) {
        V(&shutdown_mutex);
    }
    V(&thread_cnt_mutex);
    return ret;
}
#endif

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
#if 1
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    TU *target;
    P(&pbx_mutex);
        target = pbx->tu_list[ext];
    V(&pbx_mutex);
    return tu_dial(tu, target);
}
#endif
