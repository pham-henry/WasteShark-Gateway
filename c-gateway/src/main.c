// main.c
#include "signals.h"
#include "gateway.h"

int main(void)
{
    setup_signal_handlers();

    if (!gateway_init()) {
        return 1;
    }

    gateway_run();
    gateway_shutdown();

    return 0;
}
