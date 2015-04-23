#include "shared-application.h"

extern "C" {
#include <searpc-client.h>
#include <ccnet.h>

#include <searpc.h>
#include <seafile/seafile.h>
#include <seafile/seafile-object.h>

}
#include <pthread.h>

#include "utils/utils.h"
namespace {
const char *kAppletCommandsMQ = "applet.commands";
bool isActivate = false;

void *askActivateSynchronically(void *args) {
    Q_UNUSED(args);
    CcnetClient *sync_client = ccnet_client_new();
    const QString ccnet_dir = defaultCcnetDir();
    if (ccnet_client_load_confdir(sync_client, toCStr(ccnet_dir)) <  0) {
        return NULL;
    }

    if (ccnet_client_connect_daemon(sync_client, CCNET_CLIENT_SYNC) < 0) {
        return NULL;
    }

    CcnetMessage *syn_message;
    syn_message = ccnet_message_new(sync_client->base.id,
                                    sync_client->base.id,
                                    kAppletCommandsMQ, "syn_activate", 0);

    if (ccnet_client_send_message(sync_client, syn_message) < 0) {
        ccnet_message_free(syn_message);
        return NULL;
    }

    ccnet_message_free(syn_message);

    if (ccnet_client_prepare_recv_message(sync_client, kAppletCommandsMQ) < 0) {
        return NULL;
    }
    // blocking io, but cancellable
    CcnetMessage *ack_message = ccnet_client_receive_message(sync_client);
    isActivate = g_strcmp0(ack_message->body, "ack_activate") == 0;

    ccnet_message_free(ack_message);

    g_object_unref (sync_client);
    return NULL;
}
} // anonymous namespace

bool SharedApplication::activate() {
    pthread_t thread;
    // do it in a background thread
    if (pthread_create(&thread, NULL, askActivateSynchronically, NULL) != 0) {
        return false;
    }
    // keep wait for timeout or thread quiting
    int waiting_ms = 10;
    while (pthread_kill(thread, 0) == 0 && --waiting_ms > 0) {
        msleep(100);
    }
    // force to cancel it
    if (pthread_kill(thread, 0) == 0) {
        pthread_cancel(thread);
    }
    return isActivate;
}
