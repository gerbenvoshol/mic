/*******************************************************************************************
*
*   mic (make-it-c) - A simple and easy-to-use library to build pipelines in C
*
*   NOTES:
*       Memory footprint of this library is aproximately xxx bytes (global variables)
*
*   CONFIGURATION:
*
*   #define MIC_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define MIC_xxxx
*       If defined, the library can...
*
*   DEPENDENCIES:
*       None.
*
*   CONTRIBUTORS:
*       Ramon Santamaria:   Main developer, maintainer
*
*
*   LICENSE: MIT License
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#ifndef MIC_H
#define MIC_H

#include <stdarg.h>     // Required for: va_list
#include <stdbool.h>    // Required for: bool type

#ifndef MICAPI
    #define MICAPI   // We are building or using the library as a static library (or Linux shared library)
#endif

#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define MICAPI __declspec(dllexport)         // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define MICAPI __declspec(dllimport)         // We are using the library as a Win32 shared library (.dll)
    #endif
#endif

// Support TRACELOG macros
#ifndef TRACELOG
    #define TRACELOG(level, ...) micTraceLog(level, __VA_ARGS__)
    #define TRACELOGD(...) (void)0
#endif

// Allow custom memory allocators
#ifndef MIC_MALLOC
    #define MIC_MALLOC(sz)     malloc(sz)
#endif
#ifndef MIC_CALLOC
    #define MIC_CALLOC(n,sz)   calloc(n,sz)
#endif
#ifndef MIC_REALLOC
    #define MIC_REALLOC(n,sz)  realloc(n,sz)
#endif
#ifndef MIC_FREE
    #define MIC_FREE(p)        free(p)
#endif

#define MIC_STRING_STATIC_MAX_SIZE      2048
#define MAX_TRACELOG_MSG_LENGTH         512
#define MAX_FILEPATH_LENGTH             1024


//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Trace log level
// NOTE: Organized by priority level
typedef enum {
    MIC_LOG_ALL = 0,        // Display all logs
    MIC_LOG_TRACE,          // Trace logging, intended for internal use only
    MIC_LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    MIC_LOG_INFO,           // Info logging, used for program execution info
    MIC_LOG_WARNING,        // Warning logging, used on recoverable failures
    MIC_LOG_ERROR,          // Error logging, used on unrecoverable failures
    MIC_LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    MIC_LOG_NONE            // Disable logging
} micTraceLogLevel;

// Environment info
typedef enum {
    MIC_ENV_INFO_OS,
    MIC_ENV_INFO_OS_VERSION,
    MIC_ENV_INFO_PLATFORM,
    MIC_ENV_INFO_MACHINE_NAME,
} micEnvInfo;

// Callbacks to hook some internal functions
typedef void (*micTraceLogCallback)(int logLevel, const char *text, va_list args);  // Logging: Redirect trace log messages

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

// Log System
MICAPI void micTraceLog(int logLevel, const char *text, ...);           // Show trace log messages (LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR...)
MICAPI void micSetTraceLogLevel(int logLevel);                          // Set the current threshold (minimum) log level
MICAPI void micSetTraceLogCallback(micTraceLogCallback callback);          // Set custom trace log

// Environment
MICAPI void micSetEnvironmentFlags(unsigned int flags);                 // Setup environment config flags
MICAPI void micSetEnvironmentPath(const char *path);                    // Set environment path (added to system PATH)
MICAPI const char *micGetEnvironmentInfo(int info);                     // Get environment info: enum micEnvInfo

// Processes execution
MICAPI void micBeginProcess(const char *description, int level);        // [!] Begin a new process -> Not sure yet how use this, maybe just for logging...
MICAPI void micEndProcess(void);                                        // End current process
MICAPI void micBeginStep(const char *description, int level);           // [!] Begin a new process step -> Not sure yet how use it
MICAPI void micEndStep(void);                                           // End current step

MICAPI int micExecuteCommand(const char *command, ...);                 // Execute command line command, parameters passed as additional arguments
MICAPI int micExecuteMIC(const char *micFile);                          // Compile and execute another mic file
//MICAPI void micPrintMessage(const char *message, ...);                // [!] Probably not required -> Just use printf()

// Timming
MICAPI void micInitTimer(void);                                         // [!] Initialize internal timer -> Support multiple timers?
MICAPI double micGetTime(void);                                         // Get elapsed time in seconds since micInitTimer()
MICAPI long micGetTimeStamp(void);                                      // Get current date and time (now)
MICAPI const char *micGetTimeStampString(long timestamp);               // [!] Get timestamp as a string -> format? or just let the user manage it?
MICAPI int micWaitTime(int milliseconds);                               // Wait (sleep) a specific amount of time

// File system: Edition
MICAPI int micCreateFile(const char *fileName);                           // Create an empty file, useful for further filling
MICAPI int micDeleteFile(const char *fileName);                         // Delete an existing file
MICAPI int micRenameFile(const char *fileName, const char *newFileName);    // Rename an existing file
MICAPI int micCopyFile(const char *srcFileName, const char *dstPathFileName);   // Copy an existing file to a new path and filename
MICAPI int micMoveFile(const char *srcFileName, const char *dstpathFileName);   // Move an existing file to a new path and filename

MICAPI int micMakeDirectory(const char *dirPathName);                   // Create an empty directory
MICAPI int micDeleteDirectory(const char *dirPath);                     // Delete an existing and empty directory
MICAPI int micRenameDirectory(const char *dirPath, const char *newDirPath);     // Rename an existing directory
MICAPI int micCopyDirectory(const char *srcDirPath, const char *dstDirPath);    // Copy an existing directory to a new path
MICAPI int micMoveDirectory(const char *srcDirPath, const char *dstDirPath);    // Move an existing directory to a new path

// File system: Query
MICAPI bool micIsFileAvailable(const char *fileName);                   // Check if a file exists
MICAPI bool micIsDirectoryAvailable(const char *dirPath);               // Check if a directory path exists

MICAPI const char *micGetWorkingDirectory(void);                        // Get current working directory (uses static string)
MICAPI bool micChangeDirectory(const char *dirPath);                    // Change working directory, return true on success

MICAPI int micGetFileSize(const char *fileName);
MICAPI long micGetFileModTime(const char *fileName);                    // Get file modification time (last write time)
MICAPI bool micIsFileExtension(const char *fileName, const char *ext);  // Check file extension (including point: .png, .wav)
MICAPI const char *micGetFileExtension(const char *fileName);           // Get pointer to extension for a filename string (includes dot: '.png')
MICAPI const char *micGetFileName(const char *filePath);                // Get pointer to filename for a path string
MICAPI const char *micGetFileNameWithoutExt(const char *filePath);      // Get filename string without extension (uses static string)
MICAPI const char *micGetDirectoryPath(const char *filePath);           // Get full path for a given fileName with path (uses static string)
MICAPI const char *micGetPrevDirectoryPath(const char *dirPath);        // Get previous directory path for a given path (uses static string)
MICAPI const char *micGetFileFullPath(const char *fileName);            // Get full path for a file
MICAPI const char *micGetFileRelativePath(const char *fileName, const char *refPath);
MICAPI long micGetFileInfo(const char *fileName, int info);  // MIC_FILE_INFO_TIME_CREATION, MIC_FILE_INFO_TIME_LAST_ACCESS, MIC_FILE_INFO_TIME_LAST_WRITE

MICAPI int micGetDirectorySize(const char *dirPath);                    // Get directory byte size (for all files contained)
MICAPI char **micGetDirectoryFiles(const char *dirPath, int *count);    // Get filenames in a directory path (memory should be freed)
MICAPI void micClearDirectoryFiles(void);                               // Clear directory files paths buffers (free memory)

// String management (no UTF-8 strings, only byte chars)
// NOTE: Some strings allocate memory internally for returned strings -> REVIEW
MICAPI int micStringCopy(char *dstStr, const char *srcStr);             // Copy one string to another, returns bytes copied
MICAPI bool micStringEqual(const char *str1, const char *str2);         // Check if two string are equal
MICAPI unsigned int micStringSize(const char *str);                     // Get string size in bytes, checks for '\0' ending
MICAPI const char *micStringFormat(const char *str, ...);               // String formatting with variables (sprintf() style)
MICAPI const char *micStringSubstring(const char *str, int position, int length);          // Get a piece of a string
MICAPI char *micStringReplace(char *str, const char *replace, const char *by);             // Replace string (WARNING: memory must be freed!)
MICAPI char *micStringInsert(const char *str, const char *insert, int position);           // Insert string in a position (WARNING: memory must be freed!)
MICAPI const char *micStringJoin(const char **strList, int count, const char *delimiter);  // Join strings with delimiter
MICAPI const char **micStringSplit(const char *str, char delimiter, int *count);           // Split string into multiple strings
MICAPI void micStringAppend(char *str, const char *append, int *position);                 // Append string at specific position and move cursor!
MICAPI int micStringFindIndex(const char *str, const char *find);       // Find first string occurrence within a string
MICAPI const char *micStringToUpper(const char *str);                   // Get upper case version of provided string
MICAPI const char *micStringToLower(const char *str);                   // Get lower case version of provided string
MICAPI const char *micStringToPascal(const char *str);                  // Get Pascal case notation version of provided string
MICAPI int micStringToInteger(const char *str);                         // Get integer value from string (negative values not supported)
MICAPI bool micStringContains(const char *str, const char *contain);    // Check if a string contains another string
MICAPI bool micStringStartsWith(const char *str, const char *start);    // Check if a string starts with another prefix string

// Misc functions
MICAPI bool micSaveStorageValue(unsigned int position, int value);      // Save integer value to storage file (to defined position), returns true on success
MICAPI int micLoadStorageValue(unsigned int position);                  // Load integer value from storage file (from defined position)

MICAPI int micGetRandomValue(int min, int max);                         // Get a random value between min and max (both included)
MICAPI void micSetRandomSeed(unsigned int seed);                        // Set the seed for the random number generator

MICAPI unsigned char *micLoadFileData(const char *fileName, unsigned int *bytesRead);     // Load file data as byte array (read)
MICAPI void micUnloadFileData(unsigned char *data);                     // Unload file data allocated by LoadFileData()
MICAPI bool micSaveFileData(const char *fileName, void *data, unsigned int bytesToWrite); // Save data to file from byte array (write), returns true on success
MICAPI char *micLoadFileText(const char *fileName);                     // Load text data from file (read), returns a '\0' terminated string
MICAPI void micUnloadFileText(char *text);                              // Unload file text data allocated by LoadFileText()
MICAPI bool micSaveFileText(const char *fileName, char *text);          // Save text data to file (write), string must be '\0' terminated, returns true on success

MICAPI int micZipFile(const char *srcFileName, const char *dstFileName);    // Compress file into a .zip
MICAPI int micZipDirectory(const char *srcPath, const char *dstFileName);   // Compress directory into a .zip

MICAPI unsigned char *micCompressData(unsigned char *data, int dataLength, int *compDataLength);        // Compress data (DEFLATE algorithm)
MICAPI unsigned char *micDecompressData(unsigned char *compData, int compDataLength, int *dataLength);  // Decompress data (DEFLATE algorithm)

#ifdef __cplusplus
}
#endif

#endif // MIC_H


/***********************************************************************************
*
*   MIC IMPLEMENTATION
*
************************************************************************************/

#if defined(MIC_IMPLEMENTATION)

#include <stdio.h>
#include <stdlib.h>                 // Required for: setenv(), malloc, free, rand, srand
#include <string.h>                 // Required for: strlen(), strcmp(), strcpy(), strcat()
#include <stdarg.h>                 // Required for: va_list, va_start(), va_end()
#include <math.h>                   // Required for: sinf(), cosf(), sqrtf()
#include <time.h>                   // Required for: time(), localtime()
#include <ctype.h>                  // Required for: toupper(), tolower()
#include <stdbool.h>                // Required for: bool, true, false
#include <errno.h>                  // Required for: errno, error constants
#include <limits.h>                 // Required for: INT_MAX, PATH_MAX

#if !defined(_WIN32)
    #include <unistd.h>             // Required for: access(), execv()
    #include <dirent.h>             // Required for: opendir(), readdir(), closedir()
#endif

#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #include <io.h>                 // Required for: _access() [Used in FileExists()]
#else
    #include <unistd.h>             // Required for: getch(), chdir() (POSIX), access()
    #define GETCWD getcwd
    #define CHDIR chdir
#endif

#if defined(_WIN32)
    #if defined(__cplusplus)
    extern "C" {        // Prevents name mangling of functions
    #endif
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
    #if defined(__cplusplus)
    }
    #endif
#elif defined(__linux__)
    #if _POSIX_C_SOURCE < 199309L
        #undef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
    #endif
    #include <sys/time.h>               // Required for: timespec
    #include <time.h>                   // Required for: clock_gettime()

    #include <math.h>                   // Required for: sqrtf(), atan2f()
#endif
#if defined(__APPLE__)                  // macOS also defines __MACH__
    #include <mach/clock.h>             // Required for: clock_get_time()
    #include <mach/mach.h>              // Required for: mach_timespec_t
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition (internal)
//----------------------------------------------------------------------------------
typedef struct micData {
    int logTypeLevel;
    micTraceLogCallback traceLog;
    
    struct {
        unsigned long long int base;
        double previous;
    } Time;
} micData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
micData MIC = { 0 };

//----------------------------------------------------------------------------------
// Module internal Functions Declaration
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Log System
//----------------------------------------------------------------------------------

// Show trace log messages (LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR...)
void micTraceLog(int logLevel, const char *text, ...)
{
    // Message has level below current threshold, don't emit
    if (logLevel < MIC.logTypeLevel) return;

    va_list args;
    va_start(args, text);

    if (MIC.traceLog)
    {
        MIC.traceLog(logLevel, text, args);
        va_end(args);
        return;
    }

    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };

    switch (logLevel)
    {
        case MIC_LOG_TRACE: strcpy(buffer, "TRACE: "); break;
        case MIC_LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        case MIC_LOG_INFO: strcpy(buffer, "INFO: "); break;
        case MIC_LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case MIC_LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case MIC_LOG_FATAL: strcpy(buffer, "FATAL: "); break;
        default: break;
    }

    strcat(buffer, text);
    strcat(buffer, "\n");
    vprintf(buffer, args);

    va_end(args);

    if (logLevel == MIC_LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program
}

// Set the current threshold (minimum) log level
void micSetTraceLogLevel(int logLevel)
{
    MIC.logTypeLevel = logLevel;
}

// Set custom trace log
void micSetTraceLogCallback(micTraceLogCallback callback)
{
    MIC.traceLog = callback;
}

// Environment
//----------------------------------------------------------------------------------

// Setup environment config flags
void micSetEnvironmentFlags(unsigned int flags)
{
    //setenv("VARNAME", "VARVALUE", 1);   // overwrite = 1
    //int setenv(const char *envname, const char *envval, int overwrite);
    //EINVAL: The name argument is a null pointer, points to an empty string, or points to a string containing an '=' character.
    //ENOMEM: Insufficient memory was available to add a variable or its value to the environment.
    
    //char *getenv(const char *name);
    //If the specified name cannot be found in the environment of the calling process, a null pointer shall be returned.
}

// Set environment path (added to system PATH)
void micSetEnvironmentPath(const char *path)
{
    if (path == NULL) return;
    
    #if defined(_WIN32)
        // On Windows, use _putenv
        char envVar[2048] = { 0 };
        snprintf(envVar, sizeof(envVar) - 1, "PATH=%s", path);
        _putenv(envVar);
    #else
        // On Unix/Linux, use setenv
        setenv("PATH", path, 1);  // 1 = overwrite
    #endif
}

// Get environment info: enum micEnvInfo
const char *micGetEnvironmentInfo(int info)
{
    static char buffer[256] = { 0 };
    
    switch (info)
    {
        case MIC_ENV_INFO_OS:
        {
            #if defined(_WIN32)
                strcpy(buffer, "Windows");
            #elif defined(__linux__)
                strcpy(buffer, "Linux");
            #elif defined(__APPLE__)
                strcpy(buffer, "macOS");
            #elif defined(__FreeBSD__)
                strcpy(buffer, "FreeBSD");
            #elif defined(__unix__)
                strcpy(buffer, "Unix");
            #else
                strcpy(buffer, "Unknown");
            #endif
        } break;
        case MIC_ENV_INFO_OS_VERSION:
        {
            strcpy(buffer, "Unknown");
            // OS version detection would require platform-specific code
        } break;
        case MIC_ENV_INFO_PLATFORM:
        {
            #if defined(_WIN64)
                strcpy(buffer, "x64");
            #elif defined(_WIN32)
                strcpy(buffer, "x86");
            #elif defined(__x86_64__) || defined(__amd64__)
                strcpy(buffer, "x64");
            #elif defined(__i386__)
                strcpy(buffer, "x86");
            #elif defined(__aarch64__) || defined(__arm64__)
                strcpy(buffer, "ARM64");
            #elif defined(__arm__)
                strcpy(buffer, "ARM");
            #else
                strcpy(buffer, "Unknown");
            #endif
        } break;
        case MIC_ENV_INFO_MACHINE_NAME:
        {
            #if defined(_WIN32)
                const char *name = getenv("COMPUTERNAME");
                if (name) strcpy(buffer, name);
                else strcpy(buffer, "Unknown");
            #else
                const char *name = getenv("HOSTNAME");
                if (name) strcpy(buffer, name);
                else strcpy(buffer, "Unknown");
            #endif
        } break;
        default: strcpy(buffer, "Unknown"); break;
    }
    
    return buffer;
}

// Processes execution
//----------------------------------------------------------------------------------

// [!] Begin a new process -> Not sure yet how use this, maybe just for logging...
void micBeginProcess(const char *description, int level)
{
    // Simple implementation: just log the process start
    micTraceLog(level, "BEGIN PROCESS: %s", description);
}

// End current process
void micEndProcess(void)
{
    // Simple implementation: just log the process end
    micTraceLog(MIC_LOG_INFO, "END PROCESS");
}

// [!] Begin a new process step -> Not sure yet how use it
void micBeginStep(const char *description, int level)
{
    // Simple implementation: just log the step start
    micTraceLog(level, "  BEGIN STEP: %s", description);
}

// End current step
void micEndStep(void)
{
    // Simple implementation: just log the step end
    micTraceLog(MIC_LOG_INFO, "  END STEP");
}

// Execute command line command, parameters passed as additional arguments
int micExecuteCommand(const char *command, ...)
{
    char fullCmd[512] = { 0 };
    
    va_list args;
    va_start(args, command);
    
    vsnprintf(fullCmd, sizeof(fullCmd) - 1, command, args);
    
    va_end(args);
    
    int result = system(fullCmd);
    
    return result;
}

// Compile and execute another mic file
int micExecuteMIC(const char *micFile)
{
    // This would require compiling the mic file with a C compiler and executing it
    // For now, return error as this is a complex feature
    micTraceLog(MIC_LOG_WARNING, "micExecuteMIC() not yet implemented");
    return -1;
}

// Timming
//----------------------------------------------------------------------------------
// [!] Initialize internal timer -> Support multiple timers?
void micInitTimer(void)
{
// Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
// However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
// High resolutions can also prevent the CPU power management system from entering power-saving modes.
// Setting a higher resolution does not improve the accuracy of the high-resolution performance counter.
#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    timeBeginPeriod(1);                 // Setup high-resolution timer to 1ms (granularity of 1-2 ms)
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_DRM)
    struct timespec now = { 0 };

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        MIC.Time.base = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;
    }
    else TRACELOG(LOG_WARNING, "TIMER: Hi-resolution timer not available");
#endif

    MIC.Time.previous = micGetTime();     // Get time as double
}

// Get elapsed time in seconds since micInitTimer()
// Time measure returned are milliseconds
double micGetTime(void)
{
    double time = 0;

#if defined(_WIN32)
    unsigned long long int clockFrequency, currentTime;

    QueryPerformanceFrequency(&clockFrequency);     // BE CAREFUL: Costly operation!
    QueryPerformanceCounter(&currentTime);

    time = (double)currentTime/clockFrequency*1000.0f;  // Time in miliseconds
#endif

#if defined(__linux__)
    // NOTE: Only for Linux-based systems
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;     // Time in nanoseconds

    time = ((double)nowTime/1000000.0);     // Time in miliseconds
#endif

#if defined(__APPLE__)
    //#define CLOCK_REALTIME  CALENDAR_CLOCK    // returns UTC time since 1970-01-01
    //#define CLOCK_MONOTONIC SYSTEM_CLOCK      // returns the time since boot time

    clock_serv_t cclock;
    mach_timespec_t now;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);

    // NOTE: OS X does not have clock_gettime(), using clock_get_time()
    clock_get_time(cclock, &now);
    mach_port_deallocate(mach_task_self(), cclock);
    unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;     // Time in nanoseconds

    time = ((double)nowTime/1000000.0);     // Time in miliseconds
#endif

    return time;
}

// Get current date and time (now)
long micGetTimeStamp(void)
{
    return (long)time(NULL);
}

// [!] Get timestamp as a string -> format? or just let the user manage it?
const char *micGetTimeStampString(long timestamp)
{
    static char timeStr[64] = { 0 };
    
    time_t rawTime = (time_t)timestamp;
    struct tm *timeInfo = localtime(&rawTime);
    
    if (timeInfo != NULL)
    {
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);
    }
    else
    {
        strcpy(timeStr, "Invalid timestamp");
    }
    
    return timeStr;
}

// Wait (sleep) a specific amount of time
int micWaitTime(int milliseconds)
{
    int ms = milliseconds;  // Local variable for modifications
    
#if defined(SUPPORT_BUSY_WAIT_LOOP)
    double previousTime = micGetTime();
    double currentTime = 0.0;

    // Busy wait loop
    while ((currentTime - previousTime) < ms/1000.0f) currentTime = micGetTime();
#else
    #if defined(SUPPORT_PARTIALBUSY_WAIT_LOOP)
        double busyWait = ms*0.05;     // NOTE: We are using a busy wait of 5% of the time
        ms -= (float)busyWait;
    #endif

    // System halt functions
    #if defined(_WIN32)
        Sleep((unsigned int)ms);
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__EMSCRIPTEN__)
        struct timespec req = { 0 };
        time_t sec = (int)(ms/1000.0f);
        ms -= (sec*1000);
        req.tv_sec = sec;
        req.tv_nsec = ms*1000000L;

        // NOTE: Use nanosleep() on Unix platforms... usleep() it's deprecated.
        while (nanosleep(&req, &req) == -1) continue;
    #endif
    #if defined(__APPLE__)
        usleep(ms*1000.0f);
    #endif

    #if defined(SUPPORT_PARTIALBUSY_WAIT_LOOP)
        double previousTime = micGetTime();
        double currentTime = 0.0;

        // Partial busy wait loop (only a fraction of the total wait time)
        while ((currentTime - previousTime) < busyWait/1000.0f) currentTime = micGetTime();
    #endif
#endif
    
    return 0;
}

// File system: Edition
//----------------------------------------------------------------------------------

// Create an empty file, useful for further filling
int micCreateFile(const char *fileName)
{
    FILE *file = fopen(fileName, "wb");
    
    if (file != NULL)
    {
        fclose(file);
        micTraceLog(MIC_LOG_INFO, "[%s] File created successfully", fileName);
        return 0;
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to create file", fileName);
        return -1;
    }
}

// Delete an existing file
int micDeleteFile(const char *fileName)
{
    int result = remove(fileName);
    
    switch (result)
    {
        case 0: micTraceLog(MIC_LOG_INFO, "[%s] File deleted successfully", fileName);
        // case ENAMETOOLONG: This error is used when either the total length of a file name is greater than PATH_MAX, or when an individual file name component has a length greater than NAME_MAX. See Limits for Files.
        // case ENOTDIR: A file that is referenced as a directory component in the file name exists, but it isn’t a directory.
        // case ELOOP: Too many symbolic links were resolved while trying to look up the file name.
        // case EACCES: micTraceLog(MIC_LOG_ERROR, "[%s] Write permission is denied for the directory", fileName); break;
        // case EBUSY: This error indicates that the file is being used by the system in such a way that it can’t be unlinked. For example, you might see this error if the file name specifies the root directory or a mount point for a file system.
        // case ENOENT: The file name to be deleted doesn’t exist.
        // case EPERM: On some systems unlink cannot be used to delete the name of a directory, or at least can only be used this way by a privileged user. To avoid such problems, use rmdir to delete directories. (On GNU/Linux and GNU/Hurd systems unlink can never delete the name of a directory.)
        // case EROFS: The directory containing the file name to be deleted is on a read-only file system and can’t be modified.
        default: micTraceLog(MIC_LOG_ERROR, "[%s] File can not be deleted", fileName); break;
    }

    return result;
}

// Rename an existing file
int micRenameFile(const char *fileName, const char *newFileName)
{
    int result = rename(fileName, newFileName);
    
    switch (result)
    {
        case 0: micTraceLog(MIC_LOG_INFO, "[%s] File renamed successfully", newFileName); break;
        // case EACCES: One of the directories containing newname or oldname refuses write permission; or newname and oldname are directories and write permission is refused for one of them.
        // case EBUSY: A directory named by oldname or newname is being used by the system in a way that prevents the renaming from working. This includes directories that are mount points for filesystems, and directories that are the current working directories of processes.
        // case ENOTEMPTY:
        // case EEXIST: The directory newname isn’t empty. GNU/Linux and GNU/Hurd systems always return ENOTEMPTY for this, but some other systems return EEXIST.
        // case EINVAL: oldname is a directory that contains newname.
        // case EISDIR: newname is a directory but the oldname isn’t.
        // case EMLINK: The parent directory of newname would have too many links (entries).
        // case ENOENT: The file oldname doesn’t exist.
        // case ENOSPC: The directory that would contain newname has no room for another entry, and there is no space left in the file system to expand it.
        // case EROFS: The operation would involve writing to a directory on a read-only file system.
        // case EXDEV: The two file names newname and oldname are on different file systems.
        default: micTraceLog(MIC_LOG_ERROR, "[%s] File can not be renamed", fileName); break;
    }
    
    return result;
}

// Copy an existing file to a new path and filename
int micCopyFile(const char *srcFileName, const char *dstPathFileName)
{
    FILE *srcFile = fopen(srcFileName, "rb");
    if (srcFile == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to open source file", srcFileName);
        return -1;
    }
    
    FILE *dstFile = fopen(dstPathFileName, "wb");
    if (dstFile == NULL)
    {
        fclose(srcFile);
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to create destination file", dstPathFileName);
        return -1;
    }
    
    // Copy file contents in chunks
    char buffer[8192];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0)
    {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, dstFile);
        if (bytesWritten != bytesRead)
        {
            fclose(srcFile);
            fclose(dstFile);
            micTraceLog(MIC_LOG_ERROR, "[%s] Write error during file copy", dstPathFileName);
            return -1;
        }
    }
    
    fclose(srcFile);
    fclose(dstFile);
    
    micTraceLog(MIC_LOG_INFO, "[%s] File copied successfully to [%s]", srcFileName, dstPathFileName);
    return 0;
}

// Move an existing file to a new path and filename
int micMoveFile(const char *srcFileName, const char *dstpathFileName)
{
    // Try to use rename first (more efficient if on same filesystem)
    int result = rename(srcFileName, dstpathFileName);
    
    if (result == 0)
    {
        micTraceLog(MIC_LOG_INFO, "[%s] File moved successfully to [%s]", srcFileName, dstpathFileName);
        return 0;
    }
    
    // If rename failed (possibly different filesystems), copy and delete
    result = micCopyFile(srcFileName, dstpathFileName);
    if (result == 0)
    {
        result = micDeleteFile(srcFileName);
        if (result == 0)
        {
            micTraceLog(MIC_LOG_INFO, "[%s] File moved successfully to [%s]", srcFileName, dstpathFileName);
            return 0;
        }
    }
    
    micTraceLog(MIC_LOG_ERROR, "[%s] Failed to move file", srcFileName);
    return -1;
}

int micCheckFileAccess()
{
    //int access(const char *filename, int how)     // how: R_OK, W_OK, X_OK, or the existence test F_OK
    // EACCES: The access specified by how is denied.
    // ENOENT: The file doesn’t exist.
    // EROFS:Write permission was requested for a file on a read-only file system.
}

// Create an empty directory
int micMakeDirectory(const char *dirPathName)
{
    // Mode: Read + Write + eXecute: S_IRWXU (User), S_IRWXG (Group), S_IRWXO (Others)
    int result = mkdir(dirPathName, S_IRWXU | S_IRWXG | S_IRWXO);
    
    //struct stat st = { 0 };
    //if (stat("/some/directory", &st) == -1)  // Check if directory exist
    
    switch (result)
    {
        case 0: micTraceLog(MIC_LOG_INFO, "[%s] Directory created successfully", dirPathName); break;
        case EEXIST: micTraceLog(MIC_LOG_WARNING, "[%s] Directory already exists", dirPathName); break;
        case EACCES: micTraceLog(MIC_LOG_ERROR, "[%s] Write permission denied by parent directory", dirPathName); break;
        case EMLINK: micTraceLog(MIC_LOG_ERROR, "[%s] Parent directory has too many links (entries)", dirPathName); break;
        case ENOSPC: micTraceLog(MIC_LOG_ERROR, "[%s] File system doesn't have enough space", dirPathName); break;
        case EROFS: micTraceLog(MIC_LOG_ERROR, "[%s] Parent directory is read-only", dirPathName); break;
        default: break;
    }
    
    return result;
}

// Delete an existing and empty directory
int micDeleteDirectory(const char *dirPath)
{
    int result = remove(dirPath);   // stdio.h
    //int result = rmdir(dirPath);  // unistd.h
    
    switch (result)
    {
        case 0: micTraceLog(MIC_LOG_INFO, "[%s] Directory deleted successfully", dirPath); break;
        case EEXIST: 
        case ENOTEMPTY: micTraceLog(MIC_LOG_ERROR, "[%s] Directory to be deleted is not empty", dirPath); break;
        default: break;
    }
    
    return result;
}

// Rename an existing directory
int micRenameDirectory(const char *dirPath, const char *newDirPath)
{
    int result = rename(dirPath, newDirPath);
    
    if (result == 0)
    {
        micTraceLog(MIC_LOG_INFO, "[%s] Directory renamed successfully to [%s]", dirPath, newDirPath);
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to rename directory", dirPath);
    }
    
    return result;
}

// Copy an existing directory to a new path
int micCopyDirectory(const char *srcDirPath, const char *dstDirPath)
{
    // This is a complex operation requiring recursive copying
    // For a minimal implementation, we'll just note it's not implemented
    micTraceLog(MIC_LOG_WARNING, "micCopyDirectory() not fully implemented - requires recursive directory traversal");
    
    // Create destination directory
    int result = micMakeDirectory(dstDirPath);
    if (result != 0 && result != EEXIST)
    {
        return -1;
    }
    
    // Would need to recursively copy all files and subdirectories
    // This requires platform-specific directory traversal code
    
    return -1;  // Not fully implemented
}

// Move an existing directory to a new path
int micMoveDirectory(const char *srcDirPath, const char *dstDirPath)
{
    // Try rename first
    int result = rename(srcDirPath, dstDirPath);
    
    if (result == 0)
    {
        micTraceLog(MIC_LOG_INFO, "[%s] Directory moved successfully to [%s]", srcDirPath, dstDirPath);
        return 0;
    }
    
    // If rename failed, would need to copy and delete recursively
    // This is complex and requires full directory traversal
    micTraceLog(MIC_LOG_WARNING, "micMoveDirectory() cross-filesystem move not fully implemented");
    return -1;
}

// File system: Query
//----------------------------------------------------------------------------------

// Check if a file exists
bool micIsFileAvailable(const char *fileName)
{
    #if defined(_WIN32)
        return (_access(fileName, 0) == 0);
    #else
        return (access(fileName, F_OK) == 0);
    #endif
}

// Check if a directory path exists
bool micIsDirectoryAvailable(const char *dirPath)
{
    struct stat statbuf;
    
    if (stat(dirPath, &statbuf) == 0)
    {
        return S_ISDIR(statbuf.st_mode);
    }
    
    return false;
}

// Get current working directory (uses static string)
const char *micGetWorkingDirectory(void)
{
    static char currentDir[MAX_FILEPATH_LENGTH] = { 0 };
    memset(currentDir, 0, MAX_FILEPATH_LENGTH);

    char *path = GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

    return path;
}

// Change working directory, return true on success
bool micChangeDirectory(const char *dirPath)
{
    int result = CHDIR(dirPath);
    
    if (result == 0)
    {
        micTraceLog(MIC_LOG_INFO, "Changed directory to: %s", dirPath);
        return true;
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "Failed to change directory to: %s", dirPath);
        return false;
    }
}

// Get file size
int micGetFileSize(const char *fileName)
{
    struct stat statbuf;
    
    if (stat(fileName, &statbuf) == 0)
    {
        return (int)statbuf.st_size;
    }
    
    return -1;
}

// Get file modification time (last write time)
long micGetFileModTime(const char *fileName)
{
    struct stat statbuf;
    
    if (stat(fileName, &statbuf) == 0)
    {
        return (long)statbuf.st_mtime;
    }
    
    return 0;
}

// Check file extension (including point: .png, .wav)
bool micIsFileExtension(const char *fileName, const char *ext)
{
    if (fileName == NULL || ext == NULL) return false;
    
    const char *fileExt = micGetFileExtension(fileName);
    
    if (fileExt != NULL)
    {
        return (strcmp(fileExt, ext) == 0);
    }
    
    return false;
}

// Get pointer to extension for a filename string (includes dot: '.png')
const char *micGetFileExtension(const char *fileName)
{
    if (fileName == NULL) return NULL;
    
    const char *dot = strrchr(fileName, '.');
    
    if (dot && dot != fileName)
    {
        return dot;
    }
    
    return NULL;
}

// Get pointer to filename for a path string
const char *micGetFileName(const char *filePath)
{
    if (filePath == NULL) return NULL;
    
    const char *lastSlash = NULL;
    const char *lastBackslash = NULL;
    
    lastSlash = strrchr(filePath, '/');
    lastBackslash = strrchr(filePath, '\\');
    
    // Get the rightmost separator
    const char *lastSep = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    
    if (lastSep != NULL)
    {
        return lastSep + 1;
    }
    
    return filePath;  // No path separator found, return the whole string
}

// Get filename string without extension (uses static string)
const char *micGetFileNameWithoutExt(const char *filePath)
{
    static char fileName[256] = { 0 };
    
    if (filePath == NULL) return NULL;
    
    const char *name = micGetFileName(filePath);
    const char *dot = strrchr(name, '.');
    
    if (dot != NULL && dot != name)
    {
        int length = (int)(dot - name);
        if (length > 255) length = 255;
        memcpy(fileName, name, length);
        fileName[length] = '\0';
    }
    else
    {
        strncpy(fileName, name, 255);
        fileName[255] = '\0';
    }
    
    return fileName;
}

// Get full path for a given fileName with path (uses static string)
const char *micGetDirectoryPath(const char *filePath)
{
    static char dirPath[MAX_FILEPATH_LENGTH] = { 0 };
    
    if (filePath == NULL) return NULL;
    
    const char *lastSlash = NULL;
    const char *lastBackslash = NULL;
    
    lastSlash = strrchr(filePath, '/');
    lastBackslash = strrchr(filePath, '\\');
    
    const char *lastSep = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    
    if (lastSep != NULL)
    {
        int length = (int)(lastSep - filePath);
        if (length > MAX_FILEPATH_LENGTH - 1) length = MAX_FILEPATH_LENGTH - 1;
        memcpy(dirPath, filePath, length);
        dirPath[length] = '\0';
    }
    else
    {
        strcpy(dirPath, ".");
    }
    
    return dirPath;
}

// Get previous directory path for a given path (uses static string)
const char *micGetPrevDirectoryPath(const char *dirPath)
{
    static char prevDirPath[MAX_FILEPATH_LENGTH] = { 0 };
    
    if (dirPath == NULL) return NULL;
    
    // Copy the path
    strncpy(prevDirPath, dirPath, MAX_FILEPATH_LENGTH - 1);
    prevDirPath[MAX_FILEPATH_LENGTH - 1] = '\0';
    
    // Remove trailing slashes
    int len = (int)strlen(prevDirPath);
    while (len > 0 && (prevDirPath[len - 1] == '/' || prevDirPath[len - 1] == '\\'))
    {
        prevDirPath[len - 1] = '\0';
        len--;
    }
    
    // Find last separator
    const char *lastSlash = strrchr(prevDirPath, '/');
    const char *lastBackslash = strrchr(prevDirPath, '\\');
    const char *lastSep = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    
    if (lastSep != NULL)
    {
        int length = (int)(lastSep - prevDirPath);
        prevDirPath[length] = '\0';
    }
    else
    {
        strcpy(prevDirPath, ".");
    }
    
    return prevDirPath;
}

// Get full path for a file
const char *micGetFileFullPath(const char *fileName)
{
    static char fullPath[MAX_FILEPATH_LENGTH] = { 0 };
    
    if (fileName == NULL) return NULL;
    
    #if defined(_WIN32)
        if (_fullpath(fullPath, fileName, MAX_FILEPATH_LENGTH) != NULL)
        {
            return fullPath;
        }
    #else
        if (realpath(fileName, fullPath) != NULL)
        {
            return fullPath;
        }
    #endif
    
    // If realpath fails, just return the input
    strncpy(fullPath, fileName, MAX_FILEPATH_LENGTH - 1);
    fullPath[MAX_FILEPATH_LENGTH - 1] = '\0';
    
    return fullPath;
}

// Get file path relative to another path
const char *micGetFileRelativePath(const char *fileName, const char *refPath)
{
    static char relativePath[MAX_FILEPATH_LENGTH] = { 0 };
    
    // This is a complex operation - for now, return simplified version
    // A full implementation would need path parsing and comparison
    
    if (fileName == NULL || refPath == NULL)
    {
        return fileName;
    }
    
    // Simplified: just return the filename if paths share common prefix
    // Full implementation would calculate actual relative path
    strncpy(relativePath, fileName, MAX_FILEPATH_LENGTH - 1);
    relativePath[MAX_FILEPATH_LENGTH - 1] = '\0';
    
    return relativePath;
}

// Get file info: MIC_FILE_INFO_TIME_CREATION, MIC_FILE_INFO_TIME_LAST_ACCESS, MIC_FILE_INFO_TIME_LAST_WRITE
long micGetFileInfo(const char *fileName, int info)
{
    struct stat statbuf;
    
    if (stat(fileName, &statbuf) == 0)
    {
        // Note: File creation time is not portable across all systems
        // Most systems only support modification and access time
        switch (info)
        {
            case 0: // MIC_FILE_INFO_TIME_CREATION - not portable
                #if defined(__APPLE__)
                    return (long)statbuf.st_birthtime;
                #else
                    return (long)statbuf.st_ctime;  // Use change time as fallback
                #endif
            case 1: // MIC_FILE_INFO_TIME_LAST_ACCESS
                return (long)statbuf.st_atime;
            case 2: // MIC_FILE_INFO_TIME_LAST_WRITE
                return (long)statbuf.st_mtime;
            default:
                return 0;
        }
    }
    
    return 0;
}

// Get directory byte size (for all files contained)
int micGetDirectorySize(const char *dirPath)
{
    // This requires recursive directory traversal
    // For now, return a simple implementation
    micTraceLog(MIC_LOG_WARNING, "micGetDirectorySize() not fully implemented - requires recursive directory traversal");
    return -1;
}

// Get filenames in a directory path (memory should be freed)
char **micGetDirectoryFiles(const char *dirPath, int *count)
{
    #if defined(_WIN32)
        // Windows implementation would use FindFirstFile/FindNextFile
        micTraceLog(MIC_LOG_WARNING, "micGetDirectoryFiles() not implemented for Windows");
        *count = 0;
        return NULL;
    #else
        // Unix/Linux implementation
        DIR *dir = opendir(dirPath);
        if (dir == NULL)
        {
            *count = 0;
            return NULL;
        }
        
        // First pass: count files
        int fileCount = 0;
        struct dirent *entry;
        
        while ((entry = readdir(dir)) != NULL)
        {
            // Skip . and ..
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                fileCount++;
            }
        }
        
        // Allocate array for file names
        if (fileCount > 10000) fileCount = 10000;  // Sanity check to prevent overflow
        
        char **files = (char **)MIC_MALLOC(fileCount * sizeof(char *));
        if (files == NULL)
        {
            closedir(dir);
            *count = 0;
            return NULL;
        }
        
        // Initialize array to NULL
        for (int i = 0; i < fileCount; i++)
        {
            files[i] = NULL;
        }
        
        // Second pass: store file names
        rewinddir(dir);
        int index = 0;
        
        while ((entry = readdir(dir)) != NULL && index < fileCount)
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                files[index] = (char *)MIC_MALLOC(strlen(entry->d_name) + 1);
                if (files[index] != NULL)
                {
                    strcpy(files[index], entry->d_name);
                    index++;
                }
                else
                {
                    // Allocation failed, cleanup and return what we have so far
                    *count = index;
                    closedir(dir);
                    return files;
                }
            }
        }
        
        closedir(dir);
        *count = index;
        return files;
    #endif
}

// Clear directory files paths buffers (free memory)
void micClearDirectoryFiles(void)
{
    // Note: This function is designed to work with a global buffer approach
    // However, micGetDirectoryFiles currently allocates memory that must be freed by the caller
    // This function is kept for API compatibility but is essentially a no-op
    // Users should manually free the result from micGetDirectoryFiles
    micTraceLog(MIC_LOG_INFO, "micClearDirectoryFiles() called - memory management is caller's responsibility");
}


// String management (no UTF-8 strings, only byte chars)
// NOTE: Some strings allocate memory internally for returned strings -> REVIEW
//----------------------------------------------------------------------------------

// Copy one string to another, returns bytes copied
int micStringCopy(char *dstStr, const char *srcStr)
{
    if (dstStr == NULL || srcStr == NULL) return 0;
    
    int count = 0;
    
    while (*srcStr != '\0')
    {
        *dstStr = *srcStr;
        dstStr++;
        srcStr++;
        count++;
    }
    
    *dstStr = '\0';
    
    return count;
}

// Check if two string are equal
bool micStringEqual(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL) return false;
    
    return (strcmp(str1, str2) == 0);
}

// Get string size in bytes, checks for '\0' ending
unsigned int micStringSize(const char *str)
{
    if (str == NULL) return 0;
    
    return (unsigned int)strlen(str);
}

// String formatting with variables (sprintf() style)
const char *micStringFormat(const char *str, ...)
{
    static char buffer[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    va_list args;
    va_start(args, str);
    
    vsnprintf(buffer, MIC_STRING_STATIC_MAX_SIZE - 1, str, args);
    
    va_end(args);
    
    return buffer;
}

// Get a piece of a string
const char *micStringSubstring(const char *str, int position, int length)
{
    static char substring[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    if (str == NULL || position < 0 || length <= 0) return NULL;
    
    int strLen = (int)strlen(str);
    
    if (position >= strLen) return NULL;
    
    if (position + length > strLen)
    {
        length = strLen - position;
    }
    
    if (length > MIC_STRING_STATIC_MAX_SIZE - 1)
    {
        length = MIC_STRING_STATIC_MAX_SIZE - 1;
    }
    
    memcpy(substring, str + position, length);
    substring[length] = '\0';
    
    return substring;
}

// Replace string (WARNING: memory must be freed!)
char *micStringReplace(char *str, const char *replace, const char *by)
{
    if (str == NULL || replace == NULL || by == NULL) return NULL;
    
    int replaceLen = (int)strlen(replace);
    int byLen = (int)strlen(by);
    int strLen = (int)strlen(str);
    
    // Count occurrences
    int count = 0;
    char *temp = str;
    
    while ((temp = strstr(temp, replace)) != NULL)
    {
        count++;
        temp += replaceLen;
    }
    
    if (count == 0) return str;
    
    // Allocate new string
    int newLen = strLen + count * (byLen - replaceLen);
    
    // Check for negative length (could happen if replacing with shorter string)
    if (newLen < 0) newLen = 0;
    
    char *result = (char *)MIC_MALLOC(newLen + 1);
    
    if (result == NULL) return NULL;
    
    // Build new string
    char *src = str;
    char *dst = result;
    
    while (*src != '\0')
    {
        char *match = strstr(src, replace);
        
        if (match == src)
        {
            // Copy replacement
            memcpy(dst, by, byLen);
            dst += byLen;
            src += replaceLen;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    
    return result;
}

// Insert string in a position (WARNING: memory must be freed!)
char *micStringInsert(const char *str, const char *insert, int position)
{
    if (str == NULL || insert == NULL || position < 0) return NULL;
    
    int strLen = (int)strlen(str);
    int insertLen = (int)strlen(insert);
    
    if (position > strLen) position = strLen;
    
    char *result = (char *)MIC_MALLOC(strLen + insertLen + 1);
    
    if (result == NULL) return NULL;
    
    // Copy before position
    memcpy(result, str, position);
    
    // Copy insert
    memcpy(result + position, insert, insertLen);
    
    // Copy after position
    memcpy(result + position + insertLen, str + position, strLen - position);
    
    result[strLen + insertLen] = '\0';
    
    return result;
}

// Join strings with delimiter
const char *micStringJoin(const char **strList, int count, const char *delimiter)
{
    static char joinedStr[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    if (strList == NULL || count <= 0) return NULL;
    
    joinedStr[0] = '\0';
    int delimLen = (delimiter != NULL) ? (int)strlen(delimiter) : 0;
    int currentLen = 0;
    
    for (int i = 0; i < count; i++)
    {
        if (strList[i] != NULL)
        {
            int strLen = (int)strlen(strList[i]);
            
            if (currentLen + strLen < MIC_STRING_STATIC_MAX_SIZE - 1)
            {
                strcat(joinedStr, strList[i]);
                currentLen += strLen;
                
                if (i < count - 1 && delimiter != NULL && currentLen + delimLen < MIC_STRING_STATIC_MAX_SIZE - 1)
                {
                    strcat(joinedStr, delimiter);
                    currentLen += delimLen;
                }
            }
        }
    }
    
    return joinedStr;
}

// Split string into multiple strings
const char **micStringSplit(const char *str, char delimiter, int *count)
{
    if (str == NULL || count == NULL) return NULL;
    
    // Count delimiters
    int delimCount = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == delimiter) delimCount++;
    }
    
    *count = delimCount + 1;
    
    // Allocate array for strings
    const char **result = (const char **)MIC_MALLOC(*count * sizeof(char *));
    
    if (result == NULL)
    {
        *count = 0;
        return NULL;
    }
    
    // Split string
    int index = 0;
    const char *start = str;
    
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == delimiter)
        {
            int len = (int)(&str[i] - start);
            char *token = (char *)MIC_MALLOC(len + 1);
            
            if (token != NULL)
            {
                memcpy(token, start, len);
                token[len] = '\0';
                result[index++] = token;
            }
            
            start = &str[i + 1];
        }
    }
    
    // Add last token
    int len = (int)strlen(start);
    char *token = (char *)MIC_MALLOC(len + 1);
    
    if (token != NULL)
    {
        strcpy(token, start);
        result[index++] = token;
    }
    
    *count = index;
    
    return result;
}

// Append string at specific position and move cursor
void micStringAppend(char *str, const char *append, int *position)
{
    if (str == NULL || append == NULL || position == NULL) return;
    
    int appendLen = (int)strlen(append);
    
    memcpy(str + *position, append, appendLen);
    *position += appendLen;
    str[*position] = '\0';
}

// Find first string occurrence within a string
int micStringFindIndex(const char *str, const char *find)
{
    if (str == NULL || find == NULL) return -1;
    
    const char *found = strstr(str, find);
    
    if (found != NULL)
    {
        return (int)(found - str);
    }
    
    return -1;
}

// Get upper case version of provided string
const char *micStringToUpper(const char *str)
{
    static char upperStr[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    if (str == NULL) return NULL;
    
    int len = (int)strlen(str);
    if (len >= MIC_STRING_STATIC_MAX_SIZE) len = MIC_STRING_STATIC_MAX_SIZE - 1;
    
    for (int i = 0; i < len; i++)
    {
        upperStr[i] = (char)toupper(str[i]);
    }
    
    upperStr[len] = '\0';
    
    return upperStr;
}

// Get lower case version of provided string
const char *micStringToLower(const char *str)
{
    static char lowerStr[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    if (str == NULL) return NULL;
    
    int len = (int)strlen(str);
    if (len >= MIC_STRING_STATIC_MAX_SIZE) len = MIC_STRING_STATIC_MAX_SIZE - 1;
    
    for (int i = 0; i < len; i++)
    {
        lowerStr[i] = (char)tolower(str[i]);
    }
    
    lowerStr[len] = '\0';
    
    return lowerStr;
}

// Get Pascal case notation version of provided string
const char *micStringToPascal(const char *str)
{
    static char pascalStr[MIC_STRING_STATIC_MAX_SIZE] = { 0 };
    
    if (str == NULL) return NULL;
    
    int len = (int)strlen(str);
    if (len >= MIC_STRING_STATIC_MAX_SIZE) len = MIC_STRING_STATIC_MAX_SIZE - 1;
    
    bool capitalizeNext = true;
    int index = 0;
    
    for (int i = 0; i < len && index < MIC_STRING_STATIC_MAX_SIZE - 1; i++)
    {
        if (str[i] == ' ' || str[i] == '_' || str[i] == '-')
        {
            capitalizeNext = true;
        }
        else
        {
            if (capitalizeNext)
            {
                pascalStr[index++] = (char)toupper(str[i]);
                capitalizeNext = false;
            }
            else
            {
                pascalStr[index++] = (char)tolower(str[i]);
            }
        }
    }
    
    pascalStr[index] = '\0';
    
    return pascalStr;
}

// Get integer value from string (negative values not supported)
int micStringToInteger(const char *str)
{
    if (str == NULL) return 0;
    
    int result = 0;
    int i = 0;
    
    // Skip whitespace
    while (str[i] == ' ' || str[i] == '\t') i++;
    
    // Convert digits (with basic overflow check)
    while (str[i] >= '0' && str[i] <= '9')
    {
        int digit = str[i] - '0';
        
        // Check for overflow before multiplication
        if (result > (INT_MAX - digit) / 10)
        {
            return INT_MAX;  // Return max value on overflow
        }
        
        result = result * 10 + digit;
        i++;
    }
    
    return result;
}

// Check if a string contains another string
bool micStringContains(const char *str, const char *contain)
{
    if (str == NULL || contain == NULL) return false;
    
    return (strstr(str, contain) != NULL);
}

// Check if a string starts with another prefix string
bool micStringStartsWith(const char *str, const char *start)
{
    if (str == NULL || start == NULL) return false;
    
    int startLen = (int)strlen(start);
    
    return (strncmp(str, start, startLen) == 0);
}

// Misc functions
//----------------------------------------------------------------------------------

// Save integer value to storage file (to defined position), returns true on success
bool micSaveStorageValue(unsigned int position, int value)
{
    const char *storageFile = "storage.data";
    
    FILE *file = fopen(storageFile, "rb+");
    bool newFile = false;
    
    if (file == NULL)
    {
        file = fopen(storageFile, "wb+");
        newFile = true;
    }
    
    if (file == NULL) return false;
    
    // Seek to position
    if (fseek(file, position * sizeof(int), SEEK_SET) != 0)
    {
        fclose(file);
        return false;
    }
    
    // Write value
    size_t written = fwrite(&value, sizeof(int), 1, file);
    
    fclose(file);
    
    return (written == 1);
}

// Load integer value from storage file (from defined position)
int micLoadStorageValue(unsigned int position)
{
    const char *storageFile = "storage.data";
    
    FILE *file = fopen(storageFile, "rb");
    
    if (file == NULL) return 0;
    
    // Seek to position
    if (fseek(file, position * sizeof(int), SEEK_SET) != 0)
    {
        fclose(file);
        return 0;
    }
    
    // Read value
    int value = 0;
    size_t read = fread(&value, sizeof(int), 1, file);
    
    fclose(file);
    
    return (read == 1) ? value : 0;
}

// Get a random value between min and max (both included)
int micGetRandomValue(int min, int max)
{
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    
    return min + (rand() % (max - min + 1));
}

// Set the seed for the random number generator
void micSetRandomSeed(unsigned int seed)
{
    srand(seed);
}

// Load file data as byte array (read)
unsigned char *micLoadFileData(const char *fileName, unsigned int *bytesRead)
{
    unsigned char *data = NULL;
    *bytesRead = 0;
    
    FILE *file = fopen(fileName, "rb");
    
    if (file == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to open file", fileName);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size > 0)
    {
        data = (unsigned char *)MIC_MALLOC(size);
        
        if (data != NULL)
        {
            unsigned int count = (unsigned int)fread(data, 1, size, file);
            *bytesRead = count;
            
            if (count != size)
            {
                micTraceLog(MIC_LOG_WARNING, "[%s] File partially loaded (%u of %d bytes)", fileName, count, size);
            }
            else
            {
                micTraceLog(MIC_LOG_INFO, "[%s] File loaded successfully (%u bytes)", fileName, count);
            }
        }
        else
        {
            micTraceLog(MIC_LOG_ERROR, "[%s] Failed to allocate memory for file data", fileName);
        }
    }
    else
    {
        micTraceLog(MIC_LOG_WARNING, "[%s] File is empty", fileName);
    }
    
    fclose(file);
    
    return data;
}

// Unload file data allocated by LoadFileData()
void micUnloadFileData(unsigned char *data)
{
    if (data != NULL) MIC_FREE(data);
}

// Save data to file from byte array (write), returns true on success
bool micSaveFileData(const char *fileName, void *data, unsigned int bytesToWrite)
{
    if (data == NULL || bytesToWrite == 0) return false;
    
    FILE *file = fopen(fileName, "wb");
    
    if (file == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to open file for writing", fileName);
        return false;
    }
    
    unsigned int written = (unsigned int)fwrite(data, 1, bytesToWrite, file);
    
    fclose(file);
    
    if (written == bytesToWrite)
    {
        micTraceLog(MIC_LOG_INFO, "[%s] File saved successfully (%u bytes)", fileName, written);
        return true;
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to write all data (%u of %u bytes)", fileName, written, bytesToWrite);
        return false;
    }
}

// Load text data from file (read), returns a '\0' terminated string
char *micLoadFileText(const char *fileName)
{
    char *text = NULL;
    
    FILE *file = fopen(fileName, "rt");
    
    if (file == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to open text file", fileName);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size > 0)
    {
        text = (char *)MIC_MALLOC(size + 1);
        
        if (text != NULL)
        {
            unsigned int count = (unsigned int)fread(text, 1, size, file);
            text[count] = '\0';
            
            if (count != size)
            {
                micTraceLog(MIC_LOG_WARNING, "[%s] Text file partially loaded (%u of %d bytes)", fileName, count, size);
            }
            else
            {
                micTraceLog(MIC_LOG_INFO, "[%s] Text file loaded successfully (%u bytes)", fileName, count);
            }
        }
        else
        {
            micTraceLog(MIC_LOG_ERROR, "[%s] Failed to allocate memory for text file", fileName);
        }
    }
    else
    {
        micTraceLog(MIC_LOG_WARNING, "[%s] Text file is empty", fileName);
        text = (char *)MIC_MALLOC(1);
        if (text != NULL) text[0] = '\0';
    }
    
    fclose(file);
    
    return text;
}

// Unload file text data allocated by LoadFileText()
void micUnloadFileText(char *text)
{
    if (text != NULL) MIC_FREE(text);
}

// Save text data to file (write), string must be '\0' terminated, returns true on success
bool micSaveFileText(const char *fileName, char *text)
{
    if (text == NULL) return false;
    
    FILE *file = fopen(fileName, "wt");
    
    if (file == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to open file for writing text", fileName);
        return false;
    }
    
    int len = (int)strlen(text);
    int written = (int)fwrite(text, 1, len, file);
    
    fclose(file);
    
    if (written == len)
    {
        micTraceLog(MIC_LOG_INFO, "[%s] Text file saved successfully (%d bytes)", fileName, written);
        return true;
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "[%s] Failed to write all text (%d of %d bytes)", fileName, written, len);
        return false;
    }
}

// Compress file into a .zip
int micZipFile(const char *srcFileName, const char *dstFileName)
{
    // ZIP file creation requires external library (like miniz or zlib)
    // or a full ZIP format implementation
    micTraceLog(MIC_LOG_WARNING, "micZipFile() not implemented - requires ZIP library");
    return -1;
}

// Compress directory into a .zip
int micZipDirectory(const char *srcPath, const char *dstFileName)
{
    // ZIP directory creation requires external library and recursive directory traversal
    micTraceLog(MIC_LOG_WARNING, "micZipDirectory() not implemented - requires ZIP library");
    return -1;
}

// Compress data (DEFLATE algorithm)
unsigned char *micCompressData(unsigned char *data, int dataLength, int *compDataLength)
{
    // DEFLATE compression requires external library (zlib, miniz, etc.)
    // or a full DEFLATE implementation
    micTraceLog(MIC_LOG_WARNING, "micCompressData() not implemented - requires DEFLATE library");
    *compDataLength = 0;
    return NULL;
}

// Decompress data (DEFLATE algorithm)
unsigned char *micDecompressData(unsigned char *compData, int compDataLength, int *dataLength)
{
    // DEFLATE decompression requires external library (zlib, miniz, etc.)
    // or a full DEFLATE implementation
    micTraceLog(MIC_LOG_WARNING, "micDecompressData() not implemented - requires DEFLATE library");
    *dataLength = 0;
    return NULL;
}

#endif   // MIC_IMPLEMENTATION
