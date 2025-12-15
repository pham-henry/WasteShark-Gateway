// signals.c
#include "signals.h"

#include <stdio.h>

volatile sig_atomic_t g_keep_running = 1;

#if defined(_WIN32)
// ======================= Windows implementation =======================
//
// Uses the Win32 console control handler API to catch Ctrl+C, console close,
// logoff, shutdown, etc.

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
        return TRUE; // we handled it
    default:
        return FALSE;
    }
}

static void win_fallback_signal_handler(int sig)
{
    (void)sig;
    g_keep_running = 0;
}

void setup_signal_handlers(void)
{
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fprintf(stderr, "[PLATFORM] Failed to set console control handler\n");
        // Fallback: C runtime signal() for Ctrl+C
        signal(SIGINT, win_fallback_signal_handler);
    }
}

#else
// ==================== POSIX (Linux, macOS, etc.) ======================
//
// This covers Ubuntu, macOS, and most UNIX-like systems.
// Uses sigaction for robust signal handling.

#include <string.h>
#include <signal.h>

static void handle_signal(int sig)
{
    (void)sig;
    g_keep_running = 0;
}

void setup_signal_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("[PLATFORM] sigaction(SIGINT) failed");
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("[PLATFORM] sigaction(SIGTERM) failed");
    }
}

#endif
