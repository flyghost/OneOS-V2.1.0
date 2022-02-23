#include "notifier.h"
#include "mem_pub.h"
#include "uart_pub.h"

#if CFG_NEW_SUPP
/*
 * add_notifier - add a new function to be called when something happens.
 */
int add_notifier(struct notifier **notif, notify_func func, void *arg)
{
    struct notifier *np;

    for (np = *notif; np != 0; np = np->next) {
		if (np->func == func && np->arg == arg) {
		    return 0;		// already exist
		}
    }

    np = (struct notifier *)os_malloc(sizeof(struct notifier));
    if (np == 0) {
		os_printf("notifier struct OOM\n");
		return -1;
	}
    np->next = *notif;
    np->func = func;
    np->arg = arg;
    *notif = np;

	return 0;
}

/*
 * remove_notifier - remove a function from the list of things to
 * be called when something happens.
 */
void remove_notifier(struct notifier **notif,  notify_func func, void *arg)
{
    struct notifier *np;

    for (; (np = *notif) != 0; notif = &np->next) {
		if (np->func == func && np->arg == arg) {
		    *notif = np->next;
		    os_free(np);
		    break;
		}
    }
}

/*
 * notify - call a set of functions registered with add_notify.
 */
void notify(struct notifier *notif, int type, int val)
{
    struct notifier *np;

    while ((np = notif) != 0) {
		notif = np->next;
		(*np->func)(np->arg, type, val);
    }
}

/* called in wpa_s thread, DON'T use out scope of wpa_s thread */
int register_wlan_notifier(notify_func func, void *arg)
{
	return add_notifier(&wlan_evt_notifer, func, arg);
}

/* called in wpa_s thread, DON'T use out scope of wpa_s thread */
void remove_wlan_notifier(notify_func func, void *arg)
{
    remove_notifier(&wlan_evt_notifer, func, arg);
}
#endif
