/* Tested on 1.2.6.4236 REL of Grounded from the Microsoft XBox thing on
 * Windows.
 *
 * CC0: This work has been marked as dedicated to the public domain. */

#ifdef _WIN32
#define WIN32
#endif

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <windows.h>
#include <psapi.h>
#include <TCHAR.h>

#define _LOG_CRIT "frik: "
#define _LOG_WARN "heck: "

#define logwarn(fmt, ...) printf(_LOG_WARN fmt " -- %d %s", \
                                 __VA_ARGS__, GetLastError(), formatLastError())
#define logcrit(fmt, ...) printf(_LOG_CRIT fmt " -- %d %s", \
                                 __VA_ARGS__, GetLastError(), formatLastError())
#define ptrOffset(base, offset) (LPVOID)((INT64)base + offset)

#define EXENAME "Maine-WinGDK-Shipping.exe"

#define MAX_READ_FAILS 10

#define INTERVAL_MS 50

// TODO scale here by / 50.f might be better than 100.f?
#define unreal_to_mumble_units(unreal) (unreal / 100.f)


struct MumbleMem
{
#ifdef _WIN32
    UINT32   uiVersion;
    DWORD    uiTick;
#else
    uint32_t uiVersion;
    uint32_t uiTick;
#endif
    float    fAvatarPosition[3];
    float    fAvatarFront[3];
    float    fAvatarTop[3];
    wchar_t  name[256];
    float    fCameraPosition[3];
    float    fCameraFront[3];
    float    fCameraTop[3];
    wchar_t  identity[256];
#ifdef _WIN32
    UINT32   context_len;
#else
    uint32_t context_len;
#endif
    unsigned char context[256];
    wchar_t description[2048];
};


static char* formatLastError()
{
    static char*  nofmt  = "";
    static BOOL   didfmt = FALSE;
    static LPTSTR fmtbuf;

    if (didfmt)
        LocalFree(fmtbuf);

    // https://stackoverflow.com/a/455533
    didfmt = FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM
                           | FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,
                           GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPTSTR)&fmtbuf,
                           0,
                           NULL);

    return didfmt ? fmtbuf : nofmt;
}


static BOOL matchProcessByExe(HWND hwnd, LPARAM found)
{
    DWORD pid;
    HANDLE proc;
    TCHAR exe[MAX_PATH];
    DWORD exelen;

    /* returns TRUE to continue enumeration */

    if (!(GetWindowThreadProcessId(hwnd, &pid))) {
        logwarn("GetWindowThreadProcessId");
        return TRUE;
    }

    if (!(proc = OpenProcess(  PROCESS_QUERY_INFORMATION
                             | PROCESS_VM_READ,
                             FALSE /* FALSE for no inherit */,
                             pid))) {
        logwarn("OpenProcess %d", pid);
        return TRUE;
    }

    if (!(exelen = GetProcessImageFileName(proc, exe, MAX_PATH))) {
        logwarn("GetProcessImageFileName %d", pid);
        CloseHandle(proc);
        return TRUE;
    }

    if (!_tcsstr(exe, EXENAME)) {
        CloseHandle(proc);
        return TRUE;
    }

    *(HANDLE*)found = proc;

    /* break enumeration, do not close proc as it's now the
     * caller's responsibility */

    return FALSE;
}


static HMODULE findProc()
{
    HMODULE proc = NULL;

    /* sets proc on success in matchProcessByExe */
    EnumWindows(matchProcessByExe, (LPARAM)&proc);

    return proc;
}


/* Doesn't enumerate over all modules -- only has room for one item. But the
 * first module in the process should be the exe? */
HMODULE exeModuleAddress(HANDLE proc)
{
    HMODULE address;
    DWORD _unused;

    if (EnumProcessModules(proc, &address, sizeof(address), &_unused))
        return address;
    else
        return NULL;
}


struct f3 { float x, y, z; };

static float f3len(struct f3 f)
{
    return sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
}

/* vector triples in game are described here as xyz
 *   x: south low, north high
 *   y: west low, east high
 *   z: altitude, high toward sky, goes up when you jump */
struct GroundedCam
{
    /* 000 */ struct f3 top;   char _unused0[4];
    /* 010 */ struct f3 front; char _unused1[4];
    /* 020 */ char _unused2[16 * 7];
    /* 090 */ struct f3 pos;
};


int main()
{
    HANDLE proc;
    HMODULE baseAddress;
    HANDLE mumbleLinkFile;
    struct MumbleMem* mm;
    struct GroundedCam cam;
    INT8 cantReadGood = MAX_READ_FAILS - 1;

    if (!(proc = findProc()))
    {
        logcrit("couldn't find and open " EXENAME);
        return 1;
    }

    if (!(baseAddress = exeModuleAddress(proc)))
    {
        logcrit("failed to get exe base address");
        return 1;
    }

    if (!(mumbleLinkFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink")))
    {
        logcrit("mumble should be running with link plugin enabled? OpenFileMappingW");
        return 1;
    }

    if (!(mm = MapViewOfFile(mumbleLinkFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct MumbleMem))))
    {
        CloseHandle(mumbleLinkFile);
        logcrit("MapViewOfFile");
        return 1;
    }

    wcsncpy(mm->name, L"Grounded", 256);
    wcsncpy(mm->description, L"woo hoo!", 2048);
    mm->uiVersion = 2;

    while (cantReadGood < MAX_READ_FAILS)
    {

        LPVOID p = baseAddress;

        if (   !ReadProcessMemory(proc, ptrOffset(p, 0x059290F8), (&p), sizeof(p), NULL)
            || !ReadProcessMemory(proc, ptrOffset(p, 0x0), (&p), sizeof(p), NULL)
            || !ReadProcessMemory(proc, ptrOffset(p, 0x8), (&p), sizeof(p), NULL)
            || !ReadProcessMemory(proc, ptrOffset(p, 0x700), (&cam), sizeof(cam), NULL))
        {
            logcrit("ReadProcessMemory %p %p", baseAddress, p);
            cantReadGood++;
            continue;
        }

        cantReadGood = 0;

        printf("top x %f y %f 2 %f  ", cam.top.x, cam.top.y, cam.top.z);
        printf("fnt x %f y %f 2 %f  ", cam.front.x, cam.front.y, cam.front.z);
        printf("pos x %f y %f z %f\n", cam.pos.x, cam.pos.y, cam.pos.z);

        mm->fAvatarTop[0] = mm->fCameraTop[0] = -cam.top.x;
        mm->fAvatarTop[1] = mm->fCameraTop[1] = cam.top.z;
        mm->fAvatarTop[2] = mm->fCameraTop[2] = -cam.top.y;

        mm->fAvatarFront[0] = mm->fCameraFront[0] = -cam.front.x;
        mm->fAvatarFront[1] = mm->fCameraFront[1] = cam.front.z;
        mm->fAvatarFront[2] = mm->fCameraFront[2] = -cam.front.y;

        mm->fAvatarPosition[0] = mm->fCameraPosition[0] = unreal_to_mumble_units(cam.pos.x);
        mm->fAvatarPosition[1] = mm->fCameraPosition[1] = unreal_to_mumble_units(cam.pos.z);
        mm->fAvatarPosition[2] = mm->fCameraPosition[2] = unreal_to_mumble_units(cam.pos.y);

        mm->uiTick++;

        Sleep(INTERVAL_MS);

    }

    if (!CloseHandle(proc))
        logwarn("CloseHandle(proc)");

    if (!CloseHandle(mumbleLinkFile))
        logwarn("CloseHandle(mumbleLinkFile)");

    return 0;
}
