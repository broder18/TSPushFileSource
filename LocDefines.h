#pragma once

#ifndef WINVER                  // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501     // Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600        // Change this to the appropriate value to target other versions of IE.
#endif

#include <windows.h>

// set this to !0 to enable debugging messages
#define DBG_MSG     0
