// signals.c
// Cross-platform signal handling for graceful shutdown

#include "signals.h"  // Contains g_keep_running declaration
#include <stdio.h>    // For fprintf()

// Global variable that controls the main loop
// 'volatile' tells the compiler: "this variable can change unexpectedly"
//   This is important because signals can interrupt the program at any time
//   Without volatile, the compiler might optimize away checks of this variable
// 'sig_atomic_t' is a special integer type that can be safely read/written
//   even when a signal interrupts the program
// Initialized to 1 (true) - program should keep running
volatile sig_atomic_t g_keep_running = 1;

// Preprocessor directive: compile different code for Windows vs. other platforms
#if defined(_WIN32)
// ======================= Windows implementation =======================

// Preprocessor macro: tells Windows.h to exclude rarely-used stuff
// This makes compilation faster
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // Windows API header

// Windows-specific handler function
// WINAPI is a calling convention (how function parameters are passed)
// BOOL is Windows boolean type (TRUE or FALSE)
// DWORD is Windows unsigned integer type
static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
    // switch statement: checks the value of ctrl_type
    // Similar to if/else but cleaner for multiple conditions
    switch (ctrl_type) {
    // Multiple case labels can share the same code
    // These are different events that should stop the program
    case CTRL_C_EVENT:        // Ctrl+C pressed
    case CTRL_BREAK_EVENT:    // Ctrl+Break pressed
    case CTRL_CLOSE_EVENT:    // Console window closed
    case CTRL_LOGOFF_EVENT:   // User logging off
    case CTRL_SHUTDOWN_EVENT: // System shutting down
        // Set flag to 0 (false) - tells main loop to exit
        g_keep_running = 0;
        return TRUE;  // We handled the event
    default:
        // For any other event type, we didn't handle it
        return FALSE;
    }
}

// Fallback handler if Windows API fails
// Standard C signal handler (simpler, less reliable on Windows)
static void win_fallback_handler(int sig)
{
    (void)sig;  // Mark parameter as unused
    g_keep_running = 0;  // Set flag to stop program
}

// Setup signal handlers for Windows
void setup_signal_handlers(void)
{
    // SetConsoleCtrlHandler() registers our handler function
    // Parameters:
    //   console_ctrl_handler - function to call when event occurs
    //   TRUE - enable the handler
    // Returns TRUE on success, FALSE on failure
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fprintf(stderr, "[PLATFORM] Failed to set console control handler\n");
        // Fallback: use standard C signal() function for Ctrl+C
        // SIGINT is the interrupt signal (usually from Ctrl+C)
        signal(SIGINT, win_fallback_handler);
    }
}

#else
// ==================== POSIX (Linux, macOS, etc.) ======================

// Signal handler function for POSIX systems
// Parameters:
//   sig - signal number (e.g., SIGINT, SIGTERM)
//   We don't use it, so we mark it as unused
static void posix_signal_handler(int sig)
{
    (void)sig;  // Mark parameter as unused to avoid compiler warning
    g_keep_running = 0;  // Set flag to stop the main loop
}

// Setup signal handlers for POSIX systems (Linux, macOS, etc.)
void setup_signal_handlers(void)
{
    // signal() registers a function to be called when a signal is received
    // SIGINT - interrupt signal (usually from Ctrl+C)
    // SIGTERM - termination signal (from kill command or system shutdown)
    // When either signal is received, posix_signal_handler() will be called
    signal(SIGINT, posix_signal_handler);
    signal(SIGTERM, posix_signal_handler);
}

#endif
// End of platform-specific code
