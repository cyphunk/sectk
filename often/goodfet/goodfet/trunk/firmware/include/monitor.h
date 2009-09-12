

#include <signal.h>
#include <io.h>
#include <iomacros.h>


// Generic Commands

//! Overwrite all of RAM with 0xBEEF, then reboot.
void monitor_ram_pattern();
//! Return the number of contiguous bytes 0xBEEF, to measure RAM usage.
unsigned int monitor_ram_depth();

