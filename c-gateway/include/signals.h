// signals.h
#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>   // for sig_atomic_t

#ifdef __cplusplus
extern "C" {
#endif

// Global flag used by the main loop to know when to exit
extern volatile sig_atomic_t g_keep_running;

// Install signal / console handlers depending on the platform
void setup_signal_handlers(void);

#ifdef __cplusplus
}
#endif

#endif // SIGNALS_H
