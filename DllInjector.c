/*
Written by: Hinux Chau
Description: Injecting DLL to Target process using Process Id or Process name
*/
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

//Adjust token privilege of myself
BOOL SetPrivilege(void)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;
    HANDLE hSelf=GetCurrentProcess();

    if(!OpenProcessToken(
        hSelf,
        TOKEN_ALL_ACCESS,
        &hToken
    )){
        printf("OpenProcessToken Error error: %u\n", GetLastError());
        return FALSE;
    }

    if(!LookupPrivilegeValue(
        NULL,
        SE_DEBUG_NAME,
        &luid
    )){
        printf("LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount=1;
    tp.Privileges[0].Luid=luid;
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    if(!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        NULL,
        NULL
    )){
        printf("AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }
}

//Get PID by name of the process
DWORD GetPIDByName(char *pszProcessName)
{
    /**
     * @param pszProcessName pointer to an array that contains name of the specified process
     * @return PID of the specified process
    */
    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);

    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
    DWORD pid=0;
    if(Process32First(hSnapshot, &pe32)){
        while(Process32Next(hSnapshot,&pe32)){
            if(!stricmp((char*)pe32.szExeFile, pszProcessName)){       
                pid=pe32.th32ProcessID;
                break;
            }
        }
    }
    CloseHandle(hSnapshot);
    return pid;
}

//Inject dll
BOOL Inject(
    char *pszProcessName,
    char *pszPathOfDll
    )
{
    /**
     * @param pszProcessName Name of the process
     * @param pszPathOfDll Path of Dll
     * @return Status code, i.e. true stands for "successful" and FALSE stands for "failed"
    */
    DWORD pid=GetPIDByName(pszProcessName);
    if(!pid){
        printf("Target is not run!");
        return FALSE;
    }
    int len=strlen(pszPathOfDll),wlen;
    HANDLE hProcess=OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION|PROCESS_QUERY_INFORMATION,FALSE,pid);
    if(!hProcess){
        printf("OpenProcess Failed!");
        return FALSE;
    }
    printf("Process Handle:%#x\n",hProcess);

    //Allocate free space in remote process in order to provide param for the next step.
    LPVOID lpRemoteDllName=VirtualAllocEx(hProcess,0,(DWORD)(len*sizeof(char)),MEM_COMMIT,PAGE_READWRITE);
    if(!lpRemoteDllName){
        printf("Allocate remote space failed");
        return FALSE;
    }
    printf("Remote Space:%#x\n",lpRemoteDllName);
    WriteProcessMemory(hProcess,lpRemoteDllName,(LPCVOID)pszPathOfDll,len,(SIZE_T*)&wlen);
    if(wlen!=len){
        printf("wlen!=len!");
        return FALSE;
    }

    //The base address of kernel32.dll is the same in every process.
    HMODULE hKernel32=GetModuleHandle("kernel32.dll");
    printf("hKernel32:%#x\n",hKernel32);
    FARPROC pLoadLibrary=GetProcAddress(hKernel32,"LoadLibraryA");
    printf("pLoadLibrary:%#x\n",pLoadLibrary);
    HANDLE hRemoteThread=CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)pLoadLibrary,(LPVOID)lpRemoteDllName,0,NULL);
    if(!hRemoteThread){
        printf("CreateRemoteThread Error! %d",GetLastError());
        return FALSE;
    }
    WaitForSingleObject(hRemoteThread,INFINITE);
    VirtualFreeEx(hProcess,lpRemoteDllName,(DWORD)(len*sizeof(char)),MEM_DECOMMIT);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);
    printf("Finished!");
    return TRUE;
}



int main(int argc, char const *argv[])
{    
    if(!argv[1] || !argv[2]){
        printf("Format: DllInjector.exe [PROCESS_NAME] [DLL_PATH]\n");
        system("pause");
    }
    if(!SetPrivilege()){
        system("pause");
        return -1;
    }

    char *DllPath=(char*)argv[2];
    char *ProcessName=(char*)argv[1];
    Inject(ProcessName,DllPath);

    return 0;
}
