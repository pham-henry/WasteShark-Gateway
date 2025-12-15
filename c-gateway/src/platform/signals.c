// signals.c
#include "signals.h"
#include <stdio.h>

volatile sig_atomic_t g_keep_running = 1;

#if defined(_WIN32)
// ======================= Windows implementation =======================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
    switch (ctrl_type) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        g_keep_running = 0;
        return TRUE;
    default:
        return FALSE;
    }
}

static void win_fallback_handler(int sig)
{
    (void)sig;
    g_keep_running = 0;
}

void setup_signal_handlers(void)
{
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fprintf(stderr, "[PLATFORM] Failed to set console control handler\n");
        // Fallback: standard C signal for Ctrl+C
        signal(SIGINT, win_fallback_handler);
    }
}

#else
// ==================== POSIX (Linux, macOS, etc.) ======================

static void posix_signal_handler(int sig)
{
    (void)sig;
    g_keep_running = 0;
}

void setup_signal_handlers(void)
{
    // For an intro course, simple signal() is fine.
    signal(SIGINT, posix_signal_handler);
    signal(SIGTERM, posix_signal_handler);
}

#endif
