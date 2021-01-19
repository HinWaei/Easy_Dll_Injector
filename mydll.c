#include <windows.h>
#pragma 

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            MessageBox(NULL,"Process Attach","Hello!",MB_OK);
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            MessageBox(NULL,"Thread Attach","Hello!",MB_OK);
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            MessageBox(NULL,"Thread detach","Hello!",MB_OK);
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            MessageBox(NULL,"Process Detach","Hello!",MB_OK);
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
