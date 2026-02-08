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
#define MAX_DIRECTORY_FILES             10000    // Maximum files to list in a directory (prevents overflow)
#define MAX_ZIP_ENTRIES                 1000     // Maximum files in a ZIP archive (memory limit)

// Bioinformatics workflow constants
#define MIC_MAX_SAMPLES                 1000     // Maximum number of samples in a workflow
#define MIC_MAX_PATTERN_LENGTH          512      // Maximum length of file patterns
#define MIC_MAX_CONFIG_ENTRIES          256      // Maximum configuration entries
#define MIC_MAX_CSV_FIELDS              256      // Maximum fields per CSV row
#define MIC_MAX_CSV_ROWS                100000   // Maximum rows in a CSV file


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

// Bioinformatics workflow structures
// Configuration entry
typedef struct {
    char key[256];
    char value[1024];
} micConfigEntry;

// Configuration structure
typedef struct {
    micConfigEntry entries[MIC_MAX_CONFIG_ENTRIES];
    int count;
} micConfig;

// Sample list
typedef struct {
    char **samples;
    int count;
} micSampleList;

// CSV structures
typedef struct {
    char **fields;
    int fieldCount;
} micCSVRow;

typedef struct {
    micCSVRow *rows;
    int rowCount;
    char **headers;
    int headerCount;
    char delimiter;
} micCSVFile;

// Workflow context
typedef struct {
    micConfig config;
    micSampleList samples;
    char workDir[MAX_FILEPATH_LENGTH];
    char **expandedFiles;
    int expandedCount;
} micWorkflow;

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

// Advanced Pipeline Functions
MICAPI bool micStepIsComplete(const char *stepName);                    // Check if a step was completed before
MICAPI void micMarkStepComplete(const char *stepName);                  // Mark a step as complete
MICAPI bool micSkipStepIfComplete(const char *stepName);                // Skip step if already completed, returns true if skipped
MICAPI bool micWaitForFile(const char *fileName, int timeoutMs);        // Wait for file to exist, with timeout in milliseconds
MICAPI bool micWaitForFiles(const char **fileNames, int count, int timeoutMs);  // Wait for multiple files
MICAPI bool micCheckDependencies(const char **fileNames, int count);    // Check if all dependency files exist
MICAPI void micClearStepState(const char *stepName);                    // Clear completion state for a step
MICAPI void micClearAllStepStates(void);                                // Clear all step completion states

// HPC Features - SLURM Integration
MICAPI int micSlurmSubmitJob(const char *scriptFile, const char *jobName, const char *options);  // Submit job to SLURM, returns job ID or -1
MICAPI int micSlurmJobStatus(int jobId);                                // Get job status: 0=running, 1=completed, 2=failed, -1=not found
MICAPI bool micSlurmWaitForJob(int jobId, int timeoutSec);             // Wait for job completion with timeout
MICAPI bool micSlurmCancelJob(int jobId);                               // Cancel running job
MICAPI char* micSlurmGetJobOutput(int jobId, const char *outputFile);  // Read job output file, must be freed

// HPC Features - Container Support
MICAPI int micSingularityExec(const char *image, const char *command, const char *bindPaths);  // Execute command in Singularity
MICAPI int micSingularityRun(const char *image, const char *args, const char *bindPaths);      // Run Singularity container
MICAPI int micDockerRun(const char *image, const char *command, const char *volumes);          // Run Docker container
MICAPI int micContainerExec(const char *containerType, const char *image, const char *command, const char *mounts);  // Generic container exec

// HPC Features - Asynchronous Execution
MICAPI int micAsyncExecute(const char *command);                        // Execute command asynchronously, returns process ID
MICAPI bool micAsyncIsRunning(int pid);                                 // Check if async process is still running
MICAPI int micAsyncWait(int pid, int timeoutSec);                       // Wait for async process, returns exit code
MICAPI bool micAsyncCancel(int pid);                                    // Cancel/kill async process
MICAPI char* micAsyncGetOutput(int pid, const char *outputFile);       // Read output from async process

// HPC Features - Multithreading (requires -lpthread)
#if !defined(MIC_NO_THREADS)
typedef void (*micThreadFunction)(void *arg);
typedef struct micThread micThread;
typedef struct micMutex micMutex;

MICAPI micThread* micThreadCreate(micThreadFunction func, void *arg);  // Create and start thread
MICAPI bool micThreadJoin(micThread *thread, int timeoutMs);           // Wait for thread completion
MICAPI void micThreadDetach(micThread *thread);                        // Detach thread
MICAPI micMutex* micMutexCreate(void);                                 // Create mutex
MICAPI void micMutexDestroy(micMutex *mutex);                          // Destroy mutex
MICAPI void micMutexLock(micMutex *mutex);                             // Lock mutex
MICAPI void micMutexUnlock(micMutex *mutex);                           // Unlock mutex
MICAPI int micGetNumCores(void);                                        // Get number of CPU cores
#endif

// Bioinformatics Workflow Functions

// Configuration management
MICAPI int micLoadConfig(micConfig *config, const char *fileName);              // Load configuration from key=value file
MICAPI int micLoadConfigJSON(micConfig *config, const char *fileName);          // Load configuration from JSON file
MICAPI const char *micConfigGet(micConfig *config, const char *key);            // Get configuration value by key
MICAPI const char *micConfigGetDefault(micConfig *config, const char *key, const char *defaultValue);  // Get with default
MICAPI void micConfigSet(micConfig *config, const char *key, const char *value);  // Set configuration value
MICAPI void micConfigFree(micConfig *config);                                   // Free configuration memory

// Sample list management
MICAPI int micLoadSamplesJSON(micSampleList *samples, const char *fileName);    // Load samples from JSON file
MICAPI int micLoadSamplesList(micSampleList *samples, const char *fileName);    // Load samples from text file (one per line)
MICAPI int micCreateSampleList(micSampleList *samples, const char **sampleNames, int count);  // Create from array
MICAPI const char *micGetSample(micSampleList *samples, int index);             // Get sample by index
MICAPI void micFreeSampleList(micSampleList *samples);                          // Free sample list memory

// File pattern expansion (Snakemake-like expand)
MICAPI char **micExpand(const char *pattern, micSampleList *samples, int *count);  // Expand pattern with sample list
MICAPI char **micExpandWithMap(const char *pattern, const char **keys, const char **values, int mapSize, int *count);  // Expand with key-value map
MICAPI void micFreeExpandedFiles(char **files, int count);                      // Free expanded file list
MICAPI bool micAllFilesExist(char **files, int count);                          // Check if all files exist
MICAPI bool micWaitForAllFiles(char **files, int count, int timeoutMs);         // Wait for files with timeout

// Wildcard/pattern functions
MICAPI char *micExtractWildcard(const char *pattern, const char *fileName, const char *wildcardName);  // Extract wildcard value
MICAPI char *micReplaceWildcard(const char *pattern, const char *wildcardName, const char *value);     // Replace wildcard
MICAPI char **micGetWildcards(const char *pattern, int *count);                 // Get all wildcard names from pattern

// CSV file operations
MICAPI micCSVFile *micLoadCSV(const char *fileName, char delimiter, bool hasHeader);  // Load CSV file
MICAPI bool micSaveCSV(const char *fileName, micCSVFile *csv);                  // Save CSV file
MICAPI const char *micCSVGetValue(micCSVFile *csv, int row, int col);           // Get value by indices
MICAPI const char *micCSVGetValueByName(micCSVFile *csv, int row, const char *headerName);  // Get by header name
MICAPI bool micCSVAddRow(micCSVFile *csv, const char **fields, int fieldCount); // Add row
MICAPI micCSVFile *micCSVCreate(const char **headers, int headerCount, char delimiter);  // Create empty CSV
MICAPI void micFreeCSV(micCSVFile *csv);                                         // Free CSV file
MICAPI double micCSVSumColumn(micCSVFile *csv, int colIndex);                   // Sum column values
MICAPI int micCSVCountWhere(micCSVFile *csv, int colIndex, const char *value);  // Count matching rows

// Gzip/compressed file operations
MICAPI unsigned char *micLoadGzipFile(const char *fileName, unsigned int *bytesRead);  // Read gzipped file
MICAPI char *micLoadGzipFileText(const char *fileName);                         // Read gzipped text file
MICAPI bool micSaveGzipFile(const char *fileName, void *data, unsigned int size);  // Save to gzip
MICAPI bool micSaveGzipFileText(const char *fileName, const char *text);        // Save text to gzip
MICAPI bool micConcatGzipFiles(const char **srcFiles, int srcCount, const char *dstFile);  // Concatenate gzipped files

// AWK-like text processing
MICAPI double micTextFieldSum(const char *text, int fieldIndex, char delimiter);  // Sum field values
MICAPI char *micExtractField(const char *line, int fieldIndex, char delimiter);   // Extract field from line
MICAPI char *micFilterLines(const char *text, int fieldIndex, const char *condition, char delimiter);  // Filter lines
MICAPI int micCountLines(const char *text);                                      // Count lines in text
MICAPI char *micHeadLines(const char *text, int n);                              // Get first n lines
MICAPI char *micTailLines(const char *text, int n);                              // Get last n lines
MICAPI char *micSortByField(const char *text, int fieldIndex, char delimiter, bool descending, bool numeric);  // Sort lines

// Workflow context functions
MICAPI void micWorkflowInit(micWorkflow *wf);                                    // Initialize workflow
MICAPI void micWorkflowSetWorkDir(micWorkflow *wf, const char *dirPath);         // Set working directory
MICAPI int micWorkflowLoadConfig(micWorkflow *wf, const char *configFile);       // Load config
MICAPI int micWorkflowLoadSamples(micWorkflow *wf, const char *samplesFile);     // Load samples
MICAPI char **micWorkflowExpand(micWorkflow *wf, const char *pattern, int *count);  // Expand pattern
MICAPI const char *micWorkflowConfig(micWorkflow *wf, const char *key);          // Get config value
MICAPI void micWorkflowFree(micWorkflow *wf);                                    // Free resources

// Helper path functions
MICAPI char *micContainerPath(const char *containerDir, const char *containerName);  // Build container path
MICAPI char *micBuildPath(const char *workDir, const char *relativePath);        // Build output path
MICAPI bool micEnsureOutputDir(const char *filePath);                            // Ensure directory exists for output file

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

// Optional: Include zlib if available for compression functions
#if !defined(MIC_NO_ZLIB)
    #include <zlib.h>
#endif

// Optional: Include pthread if available for multithreading
#if !defined(MIC_NO_THREADS) && !defined(_WIN32)
    #include <pthread.h>
#endif

// For async process management
#if !defined(_WIN32)
    #include <sys/wait.h>               // Required for: waitpid()
    #include <signal.h>                 // Required for: kill()
#else
    #include <process.h>                // Required for: _spawnv()
#endif

#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #include <windows.h>            // Required for: FindFirstFile(), FindNextFile(), WIN32_FIND_DATA
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

// Pipeline step state entry
typedef struct micStepState {
    char name[256];
    bool completed;
    long timestamp;
    struct micStepState *next;
} micStepState;

// Thread structure (internal)
#if !defined(MIC_NO_THREADS)
struct micThread {
    #if defined(_WIN32)
        void *handle;
    #else
        pthread_t thread;
    #endif
    micThreadFunction function;
    void *arg;
    bool detached;
};

// Mutex structure (internal)
struct micMutex {
    #if defined(_WIN32)
        void *handle;
    #else
        pthread_mutex_t mutex;
    #endif
};
#endif

typedef struct micData {
    int logTypeLevel;
    micTraceLogCallback traceLog;
    
    struct {
        unsigned long long int base;
        double previous;
    } Time;
    
    struct {
        micStepState *firstStep;
        const char *stateFileName;
    } Pipeline;
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

// Helper function for recursive directory size calculation
static long long micGetDirectorySizeRecursive(const char *dirPath)
{
    long long totalSize = 0;
    
    #if defined(_WIN32)
        // Windows implementation
        WIN32_FIND_DATAA findData;
        char searchPath[MAX_FILEPATH_LENGTH];
        
        snprintf(searchPath, MAX_FILEPATH_LENGTH, "%s\\*", dirPath);
        
        HANDLE hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        
        do
        {
            // Skip . and ..
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            {
                continue;
            }
            
            char fullPath[MAX_FILEPATH_LENGTH];
            snprintf(fullPath, MAX_FILEPATH_LENGTH, "%s\\%s", dirPath, findData.cFileName);
            
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Recursively get size of subdirectory
                totalSize += micGetDirectorySizeRecursive(fullPath);
            }
            else
            {
                // Add file size
                LARGE_INTEGER fileSize;
                fileSize.LowPart = findData.nFileSizeLow;
                fileSize.HighPart = findData.nFileSizeHigh;
                totalSize += fileSize.QuadPart;
            }
        }
        while (FindNextFileA(hFind, &findData) != 0);
        
        FindClose(hFind);
    #else
        // Unix/Linux implementation
        DIR *dir = opendir(dirPath);
        if (dir == NULL)
        {
            return 0;
        }
        
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            // Skip . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            
            char fullPath[MAX_FILEPATH_LENGTH];
            snprintf(fullPath, MAX_FILEPATH_LENGTH, "%s/%s", dirPath, entry->d_name);
            
            struct stat statbuf;
            if (stat(fullPath, &statbuf) == 0)
            {
                if (S_ISDIR(statbuf.st_mode))
                {
                    // Recursively get size of subdirectory
                    totalSize += micGetDirectorySizeRecursive(fullPath);
                }
                else
                {
                    // Add file size
                    totalSize += statbuf.st_size;
                }
            }
        }
        
        closedir(dir);
    #endif
    
    return totalSize;
}

// Get directory byte size (for all files contained)
int micGetDirectorySize(const char *dirPath)
{
    if (dirPath == NULL || !micIsDirectoryAvailable(dirPath))
    {
        micTraceLog(MIC_LOG_ERROR, "micGetDirectorySize() - Invalid directory path");
        return -1;
    }
    
    long long size = micGetDirectorySizeRecursive(dirPath);
    
    // Check if size exceeds INT_MAX
    if (size > INT_MAX)
    {
        micTraceLog(MIC_LOG_WARNING, "micGetDirectorySize() - Directory size exceeds INT_MAX, returning INT_MAX");
        return INT_MAX;
    }
    
    return (int)size;
}

// Get filenames in a directory path (memory should be freed)
char **micGetDirectoryFiles(const char *dirPath, int *count)
{
    #if defined(_WIN32)
        // Windows implementation using FindFirstFile/FindNextFile
        WIN32_FIND_DATAA findData;
        char searchPath[MAX_FILEPATH_LENGTH];
        
        *count = 0;
        
        if (dirPath == NULL)
        {
            return NULL;
        }
        
        // Create search pattern
        snprintf(searchPath, MAX_FILEPATH_LENGTH, "%s\\*", dirPath);
        
        // First pass: count files
        int fileCount = 0;
        HANDLE hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return NULL;
        }
        
        do
        {
            // Skip . and ..
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
            {
                fileCount++;
            }
        }
        while (FindNextFileA(hFind, &findData) != 0);
        
        FindClose(hFind);
        
        if (fileCount == 0)
        {
            *count = 0;
            return NULL;
        }
        
        // Sanity check to prevent overflow
        if (fileCount > MAX_DIRECTORY_FILES) fileCount = MAX_DIRECTORY_FILES;
        
        // Allocate array for file names
        char **files = (char **)MIC_MALLOC(fileCount * sizeof(char *));
        if (files == NULL)
        {
            *count = 0;
            return NULL;
        }
        
        // Initialize array to NULL
        for (int i = 0; i < fileCount; i++)
        {
            files[i] = NULL;
        }
        
        // Second pass: store file names
        hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            MIC_FREE(files);
            *count = 0;
            return NULL;
        }
        
        int index = 0;
        do
        {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
            {
                if (index < fileCount)
                {
                    files[index] = (char *)MIC_MALLOC(strlen(findData.cFileName) + 1);
                    if (files[index] != NULL)
                    {
                        strncpy(files[index], findData.cFileName, strlen(findData.cFileName) + 1);
                        files[index][strlen(findData.cFileName)] = '\0';
                        index++;
                    }
                    else
                    {
                        // Allocation failed, cleanup and return what we have so far
                        FindClose(hFind);
                        *count = index;
                        return files;
                    }
                }
            }
        }
        while (FindNextFileA(hFind, &findData) != 0);
        
        FindClose(hFind);
        *count = index;
        return files;
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
        if (fileCount > MAX_DIRECTORY_FILES) fileCount = MAX_DIRECTORY_FILES;  // Sanity check to prevent overflow
        
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

// Helper structure for ZIP file creation
typedef struct {
    char fileName[256];
    unsigned int crc32;
    unsigned int compressedSize;
    unsigned int uncompressedSize;
    unsigned int headerOffset;
} micZipFileEntry;

// Helper function to calculate CRC32 (for ZIP file integrity)
static unsigned int micCalculateCRC32(const unsigned char *data, unsigned int length)
{
    unsigned int crc = 0xFFFFFFFF;
    
    // CRC32 polynomial table (standard ZIP CRC)
    static unsigned int crc_table[256];
    static bool table_initialized = false;
    
    if (!table_initialized)
    {
        for (unsigned int i = 0; i < 256; i++)
        {
            unsigned int c = i;
            for (int j = 0; j < 8; j++)
            {
                c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
            }
            crc_table[i] = c;
        }
        table_initialized = true;
    }
    
    for (unsigned int i = 0; i < length; i++)
    {
        crc = crc_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    
    return crc ^ 0xFFFFFFFF;
}

// Helper function to write little-endian integers
static void micWriteLE16(FILE *fp, unsigned short value)
{
    fputc(value & 0xFF, fp);
    fputc((value >> 8) & 0xFF, fp);
}

static void micWriteLE32(FILE *fp, unsigned int value)
{
    fputc(value & 0xFF, fp);
    fputc((value >> 8) & 0xFF, fp);
    fputc((value >> 16) & 0xFF, fp);
    fputc((value >> 24) & 0xFF, fp);
}

// Helper function to compress data as raw DEFLATE (for ZIP files)
static unsigned char *micCompressDataRawDeflate(unsigned char *data, int dataLength, int *compDataLength)
{
    #if !defined(MIC_NO_ZLIB)
        if (data == NULL || dataLength <= 0 || compDataLength == NULL)
        {
            return NULL;
        }
        
        // Allocate buffer for compressed data (use compressBound as estimate)
        unsigned long compBufferSize = compressBound(dataLength);
        unsigned char *compData = (unsigned char *)MIC_MALLOC(compBufferSize);
        if (compData == NULL)
        {
            return NULL;
        }
        
        // Initialize zlib stream for raw DEFLATE
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = dataLength;
        stream.next_in = data;
        stream.avail_out = compBufferSize;
        stream.next_out = compData;
        
        // Initialize deflate with raw DEFLATE (no zlib wrapper)
        // windowBits = -15 means raw DEFLATE without zlib header/trailer
        if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            MIC_FREE(compData);
            return NULL;
        }
        
        // Compress
        int result = deflate(&stream, Z_FINISH);
        deflateEnd(&stream);
        
        if (result != Z_STREAM_END)
        {
            MIC_FREE(compData);
            return NULL;
        }
        
        *compDataLength = (int)stream.total_out;
        return compData;
    #else
        return NULL;
    #endif
}

// Compress file into a .zip
int micZipFile(const char *srcFileName, const char *dstFileName)
{
    #if !defined(MIC_NO_ZLIB)
        if (srcFileName == NULL || dstFileName == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micZipFile() invalid parameters");
            return -1;
        }
        
        // Load source file
        unsigned int fileSize = 0;
        unsigned char *fileData = micLoadFileData(srcFileName, &fileSize);
        if (fileData == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micZipFile() failed to load source file: %s", srcFileName);
            return -1;
        }
        
        // Calculate CRC32
        unsigned int crc32 = micCalculateCRC32(fileData, fileSize);
        
        // Compress the data using raw DEFLATE (for ZIP format)
        int compressedSize = 0;
        unsigned char *compressedData = micCompressDataRawDeflate(fileData, fileSize, &compressedSize);
        if (compressedData == NULL)
        {
            micUnloadFileData(fileData);
            micTraceLog(MIC_LOG_ERROR, "micZipFile() failed to compress data");
            return -1;
        }
        
        // Extract just the filename (no path)
        const char *fileName = micGetFileName(srcFileName);
        int fileNameLen = (int)strlen(fileName);
        
        // Open destination ZIP file
        FILE *zipFile = fopen(dstFileName, "wb");
        if (zipFile == NULL)
        {
            micUnloadFileData(fileData);
            micUnloadFileData(compressedData);
            micTraceLog(MIC_LOG_ERROR, "micZipFile() failed to create ZIP file: %s", dstFileName);
            return -1;
        }
        
        // Write local file header
        unsigned int localHeaderOffset = (unsigned int)ftell(zipFile);
        fwrite("PK\x03\x04", 1, 4, zipFile);          // Local file header signature
        micWriteLE16(zipFile, 20);                    // Version needed to extract (2.0)
        micWriteLE16(zipFile, 0);                     // General purpose bit flag
        micWriteLE16(zipFile, 8);                     // Compression method (8 = DEFLATE)
        micWriteLE16(zipFile, 0);                     // File last modification time
        micWriteLE16(zipFile, 0);                     // File last modification date
        micWriteLE32(zipFile, crc32);                 // CRC-32
        micWriteLE32(zipFile, compressedSize);        // Compressed size
        micWriteLE32(zipFile, fileSize);              // Uncompressed size
        micWriteLE16(zipFile, fileNameLen);           // File name length
        micWriteLE16(zipFile, 0);                     // Extra field length
        fwrite(fileName, 1, fileNameLen, zipFile);    // File name
        
        // Write compressed data
        fwrite(compressedData, 1, compressedSize, zipFile);
        
        // Write central directory header
        unsigned int centralDirOffset = (unsigned int)ftell(zipFile);
        fwrite("PK\x01\x02", 1, 4, zipFile);          // Central directory file header signature
        micWriteLE16(zipFile, 20);                    // Version made by
        micWriteLE16(zipFile, 20);                    // Version needed to extract
        micWriteLE16(zipFile, 0);                     // General purpose bit flag
        micWriteLE16(zipFile, 8);                     // Compression method
        micWriteLE16(zipFile, 0);                     // File last modification time
        micWriteLE16(zipFile, 0);                     // File last modification date
        micWriteLE32(zipFile, crc32);                 // CRC-32
        micWriteLE32(zipFile, compressedSize);        // Compressed size
        micWriteLE32(zipFile, fileSize);              // Uncompressed size
        micWriteLE16(zipFile, fileNameLen);           // File name length
        micWriteLE16(zipFile, 0);                     // Extra field length
        micWriteLE16(zipFile, 0);                     // File comment length
        micWriteLE16(zipFile, 0);                     // Disk number start
        micWriteLE16(zipFile, 0);                     // Internal file attributes
        micWriteLE32(zipFile, 0);                     // External file attributes
        micWriteLE32(zipFile, localHeaderOffset);     // Relative offset of local header
        fwrite(fileName, 1, fileNameLen, zipFile);    // File name
        
        // Write end of central directory record
        unsigned int centralDirSize = (unsigned int)ftell(zipFile) - centralDirOffset;
        fwrite("PK\x05\x06", 1, 4, zipFile);          // End of central directory signature
        micWriteLE16(zipFile, 0);                     // Number of this disk
        micWriteLE16(zipFile, 0);                     // Disk where central directory starts
        micWriteLE16(zipFile, 1);                     // Number of central directory records on this disk
        micWriteLE16(zipFile, 1);                     // Total number of central directory records
        micWriteLE32(zipFile, centralDirSize);        // Size of central directory
        micWriteLE32(zipFile, centralDirOffset);      // Offset of start of central directory
        micWriteLE16(zipFile, 0);                     // ZIP file comment length
        
        fclose(zipFile);
        
        // Cleanup
        micUnloadFileData(fileData);
        micUnloadFileData(compressedData);
        
        micTraceLog(MIC_LOG_INFO, "Successfully created ZIP file: %s", dstFileName);
        return 0;
    #else
        micTraceLog(MIC_LOG_WARNING, "micZipFile() not available - MIC_NO_ZLIB defined");
        return -1;
    #endif
}

// Helper function to add a file to an open ZIP file stream
static int micZipAddFileToArchive(FILE *zipFile, const char *filePath, const char *archivePath, 
                                   micZipFileEntry *entry)
{
    #if !defined(MIC_NO_ZLIB)
        // Load file data
        unsigned int fileSize = 0;
        unsigned char *fileData = micLoadFileData(filePath, &fileSize);
        if (fileData == NULL)
        {
            return -1;
        }
        
        // Calculate CRC32
        unsigned int crc32 = micCalculateCRC32(fileData, fileSize);
        
        // Compress the data using raw DEFLATE (for ZIP format)
        int compressedSize = 0;
        unsigned char *compressedData = micCompressDataRawDeflate(fileData, fileSize, &compressedSize);
        if (compressedData == NULL)
        {
            micUnloadFileData(fileData);
            return -1;
        }
        
        int archivePathLen = (int)strlen(archivePath);
        
        // Store entry info (ensure null termination)
        strncpy(entry->fileName, archivePath, sizeof(entry->fileName) - 1);
        entry->fileName[sizeof(entry->fileName) - 1] = '\0';
        entry->crc32 = crc32;
        entry->compressedSize = compressedSize;
        entry->uncompressedSize = fileSize;
        entry->headerOffset = (unsigned int)ftell(zipFile);
        
        // Write local file header
        fwrite("PK\x03\x04", 1, 4, zipFile);
        micWriteLE16(zipFile, 20);
        micWriteLE16(zipFile, 0);
        micWriteLE16(zipFile, 8);
        micWriteLE16(zipFile, 0);
        micWriteLE16(zipFile, 0);
        micWriteLE32(zipFile, crc32);
        micWriteLE32(zipFile, compressedSize);
        micWriteLE32(zipFile, fileSize);
        micWriteLE16(zipFile, archivePathLen);
        micWriteLE16(zipFile, 0);
        fwrite(archivePath, 1, archivePathLen, zipFile);
        
        // Write compressed data
        fwrite(compressedData, 1, compressedSize, zipFile);
        
        // Cleanup
        micUnloadFileData(fileData);
        micUnloadFileData(compressedData);
        
        return 0;
    #else
        return -1;
    #endif
}

// Helper function to recursively add directory contents to ZIP
static int micZipAddDirectoryRecursive(FILE *zipFile, const char *dirPath, const char *basePath,
                                        micZipFileEntry *entries, int *entryCount, int maxEntries)
{
    #if !defined(MIC_NO_ZLIB)
        int fileCount = 0;
        char **files = micGetDirectoryFiles(dirPath, &fileCount);
        
        if (files == NULL)
        {
            return 0;
        }
        
        for (int i = 0; i < fileCount && *entryCount < maxEntries; i++)
        {
            char fullPath[MAX_FILEPATH_LENGTH];
            char archivePath[MAX_FILEPATH_LENGTH];
            
            #if defined(_WIN32)
                snprintf(fullPath, MAX_FILEPATH_LENGTH, "%s\\%s", dirPath, files[i]);
            #else
                snprintf(fullPath, MAX_FILEPATH_LENGTH, "%s/%s", dirPath, files[i]);
            #endif
            
            // Calculate relative path for archive
            if (basePath != NULL && strlen(basePath) > 0)
            {
                #if defined(_WIN32)
                    snprintf(archivePath, MAX_FILEPATH_LENGTH, "%s\\%s", basePath, files[i]);
                #else
                    snprintf(archivePath, MAX_FILEPATH_LENGTH, "%s/%s", basePath, files[i]);
                #endif
            }
            else
            {
                strncpy(archivePath, files[i], MAX_FILEPATH_LENGTH - 1);
                archivePath[MAX_FILEPATH_LENGTH - 1] = '\0';
            }
            
            if (micIsDirectoryAvailable(fullPath))
            {
                // Recursively add subdirectory
                micZipAddDirectoryRecursive(zipFile, fullPath, archivePath, entries, entryCount, maxEntries);
            }
            else
            {
                // Add file to archive
                if (micZipAddFileToArchive(zipFile, fullPath, archivePath, &entries[*entryCount]) == 0)
                {
                    (*entryCount)++;
                }
            }
        }
        
        // Cleanup
        for (int i = 0; i < fileCount; i++)
        {
            MIC_FREE(files[i]);
        }
        MIC_FREE(files);
        
        return 0;
    #else
        return -1;
    #endif
}

// Compress directory into a .zip
int micZipDirectory(const char *srcPath, const char *dstFileName)
{
    #if !defined(MIC_NO_ZLIB)
        if (srcPath == NULL || dstFileName == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micZipDirectory() invalid parameters");
            return -1;
        }
        
        if (!micIsDirectoryAvailable(srcPath))
        {
            micTraceLog(MIC_LOG_ERROR, "micZipDirectory() source directory does not exist: %s", srcPath);
            return -1;
        }
        
        // Open destination ZIP file
        FILE *zipFile = fopen(dstFileName, "wb");
        if (zipFile == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micZipDirectory() failed to create ZIP file: %s", dstFileName);
            return -1;
        }
        
        // Allocate entry array for tracking files
        const int maxEntries = MAX_ZIP_ENTRIES;
        micZipFileEntry *entries = (micZipFileEntry *)MIC_MALLOC(maxEntries * sizeof(micZipFileEntry));
        if (entries == NULL)
        {
            fclose(zipFile);
            micTraceLog(MIC_LOG_ERROR, "micZipDirectory() failed to allocate memory");
            return -1;
        }
        
        int entryCount = 0;
        
        // Add all files from directory recursively
        micZipAddDirectoryRecursive(zipFile, srcPath, "", entries, &entryCount, maxEntries);
        
        // Write central directory
        unsigned int centralDirOffset = (unsigned int)ftell(zipFile);
        
        for (int i = 0; i < entryCount; i++)
        {
            int fileNameLen = (int)strlen(entries[i].fileName);
            
            fwrite("PK\x01\x02", 1, 4, zipFile);
            micWriteLE16(zipFile, 20);
            micWriteLE16(zipFile, 20);
            micWriteLE16(zipFile, 0);
            micWriteLE16(zipFile, 8);
            micWriteLE16(zipFile, 0);
            micWriteLE16(zipFile, 0);
            micWriteLE32(zipFile, entries[i].crc32);
            micWriteLE32(zipFile, entries[i].compressedSize);
            micWriteLE32(zipFile, entries[i].uncompressedSize);
            micWriteLE16(zipFile, fileNameLen);
            micWriteLE16(zipFile, 0);
            micWriteLE16(zipFile, 0);
            micWriteLE16(zipFile, 0);
            micWriteLE16(zipFile, 0);
            micWriteLE32(zipFile, 0);
            micWriteLE32(zipFile, entries[i].headerOffset);
            fwrite(entries[i].fileName, 1, fileNameLen, zipFile);
        }
        
        // Write end of central directory
        unsigned int centralDirSize = (unsigned int)ftell(zipFile) - centralDirOffset;
        fwrite("PK\x05\x06", 1, 4, zipFile);
        micWriteLE16(zipFile, 0);
        micWriteLE16(zipFile, 0);
        micWriteLE16(zipFile, entryCount);
        micWriteLE16(zipFile, entryCount);
        micWriteLE32(zipFile, centralDirSize);
        micWriteLE32(zipFile, centralDirOffset);
        micWriteLE16(zipFile, 0);
        
        fclose(zipFile);
        MIC_FREE(entries);
        
        micTraceLog(MIC_LOG_INFO, "Successfully created ZIP archive with %d files: %s", entryCount, dstFileName);
        return 0;
    #else
        micTraceLog(MIC_LOG_WARNING, "micZipDirectory() not available - MIC_NO_ZLIB defined");
        return -1;
    #endif
}

// Compress data (DEFLATE algorithm)
unsigned char *micCompressData(unsigned char *data, int dataLength, int *compDataLength)
{
    #if !defined(MIC_NO_ZLIB)
        if (data == NULL || dataLength <= 0 || compDataLength == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micCompressData() invalid parameters");
            *compDataLength = 0;
            return NULL;
        }
        
        // Allocate buffer for compressed data (worst case: input size + 0.1% + 12 bytes)
        uLongf destLen = compressBound(dataLength);
        unsigned char *compData = (unsigned char *)MIC_MALLOC(destLen);
        
        if (compData == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micCompressData() failed to allocate memory");
            *compDataLength = 0;
            return NULL;
        }
        
        // Compress using zlib (level 6 = default compression)
        int result = compress2(compData, &destLen, data, dataLength, 6);
        
        if (result != Z_OK)
        {
            MIC_FREE(compData);
            micTraceLog(MIC_LOG_ERROR, "micCompressData() compression failed with error %d", result);
            *compDataLength = 0;
            return NULL;
        }
        
        *compDataLength = (int)destLen;
        micTraceLog(MIC_LOG_INFO, "Compressed %d bytes to %d bytes (%.1f%% reduction)", 
                    dataLength, *compDataLength, 100.0 * (1.0 - (double)*compDataLength / dataLength));
        
        return compData;
    #else
        micTraceLog(MIC_LOG_WARNING, "micCompressData() not available - MIC_NO_ZLIB defined");
        *compDataLength = 0;
        return NULL;
    #endif
}

// Decompress data (DEFLATE algorithm)
unsigned char *micDecompressData(unsigned char *compData, int compDataLength, int *dataLength)
{
    #if !defined(MIC_NO_ZLIB)
        if (compData == NULL || compDataLength <= 0 || dataLength == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micDecompressData() invalid parameters");
            *dataLength = 0;
            return NULL;
        }
        
        // Try with initial buffer size estimate (compressed size * 4)
        uLongf destLen = compDataLength * 4;
        unsigned char *data = (unsigned char *)MIC_MALLOC(destLen);
        
        if (data == NULL)
        {
            micTraceLog(MIC_LOG_ERROR, "micDecompressData() failed to allocate memory");
            *dataLength = 0;
            return NULL;
        }
        
        // Try decompression
        int result = uncompress(data, &destLen, compData, compDataLength);
        
        // If buffer was too small, try with larger buffer
        if (result == Z_BUF_ERROR)
        {
            MIC_FREE(data);
            destLen = compDataLength * 10;  // Try 10x size
            data = (unsigned char *)MIC_MALLOC(destLen);
            
            if (data == NULL)
            {
                micTraceLog(MIC_LOG_ERROR, "micDecompressData() failed to allocate larger buffer");
                *dataLength = 0;
                return NULL;
            }
            
            result = uncompress(data, &destLen, compData, compDataLength);
        }
        
        if (result != Z_OK)
        {
            MIC_FREE(data);
            micTraceLog(MIC_LOG_ERROR, "micDecompressData() decompression failed with error %d", result);
            *dataLength = 0;
            return NULL;
        }
        
        *dataLength = (int)destLen;
        micTraceLog(MIC_LOG_INFO, "Decompressed %d bytes to %d bytes", compDataLength, *dataLength);
        
        return data;
    #else
        micTraceLog(MIC_LOG_WARNING, "micDecompressData() not available - MIC_NO_ZLIB defined");
        *dataLength = 0;
        return NULL;
    #endif
}

// Advanced Pipeline Functions
//----------------------------------------------------------------------------------

// Internal helper to find or create a step state entry
static micStepState *micFindOrCreateStepState(const char *stepName)
{
    if (stepName == NULL) return NULL;
    
    // Search for existing step
    micStepState *current = MIC.Pipeline.firstStep;
    while (current != NULL)
    {
        if (strcmp(current->name, stepName) == 0)
        {
            return current;
        }
        current = current->next;
    }
    
    // Create new step state
    micStepState *newStep = (micStepState *)MIC_MALLOC(sizeof(micStepState));
    if (newStep == NULL) return NULL;
    
    strncpy(newStep->name, stepName, 255);
    newStep->name[255] = '\0';
    newStep->completed = false;
    newStep->timestamp = 0;
    newStep->next = MIC.Pipeline.firstStep;
    MIC.Pipeline.firstStep = newStep;
    
    return newStep;
}

// Check if a step was completed before
bool micStepIsComplete(const char *stepName)
{
    micStepState *step = micFindOrCreateStepState(stepName);
    return (step != NULL && step->completed);
}

// Mark a step as complete
void micMarkStepComplete(const char *stepName)
{
    micStepState *step = micFindOrCreateStepState(stepName);
    if (step != NULL)
    {
        step->completed = true;
        step->timestamp = micGetTimeStamp();
        micTraceLog(MIC_LOG_INFO, "Step '%s' marked as complete", stepName);
    }
}

// Skip step if already completed, returns true if skipped
bool micSkipStepIfComplete(const char *stepName)
{
    if (micStepIsComplete(stepName))
    {
        micTraceLog(MIC_LOG_INFO, "Skipping step '%s' - already completed", stepName);
        return true;
    }
    return false;
}

// Clear completion state for a step
void micClearStepState(const char *stepName)
{
    micStepState *step = micFindOrCreateStepState(stepName);
    if (step != NULL)
    {
        step->completed = false;
        step->timestamp = 0;
        micTraceLog(MIC_LOG_INFO, "Step '%s' state cleared", stepName);
    }
}

// Clear all step completion states
void micClearAllStepStates(void)
{
    micStepState *current = MIC.Pipeline.firstStep;
    while (current != NULL)
    {
        micStepState *next = current->next;
        MIC_FREE(current);
        current = next;
    }
    MIC.Pipeline.firstStep = NULL;
    micTraceLog(MIC_LOG_INFO, "All step states cleared");
}

// Wait for file to exist, with timeout in milliseconds
bool micWaitForFile(const char *fileName, int timeoutMs)
{
    if (fileName == NULL) return false;
    
    micTraceLog(MIC_LOG_INFO, "Waiting for file '%s' (timeout: %d ms)", fileName, timeoutMs);
    
    long startTime = micGetTimeStamp();
    int pollInterval = 100;  // Poll every 100ms
    
    while (true)
    {
        // Check if file exists
        if (micIsFileAvailable(fileName))
        {
            micTraceLog(MIC_LOG_INFO, "File '%s' found", fileName);
            return true;
        }
        
        // Check timeout
        long currentTime = micGetTimeStamp();
        long elapsed = (currentTime - startTime) * 1000;  // Convert to ms
        
        if (elapsed >= timeoutMs)
        {
            micTraceLog(MIC_LOG_WARNING, "Timeout waiting for file '%s'", fileName);
            return false;
        }
        
        // Wait before next poll
        micWaitTime(pollInterval);
    }
}

// Wait for multiple files
bool micWaitForFiles(const char **fileNames, int count, int timeoutMs)
{
    if (fileNames == NULL || count <= 0) return false;
    
    micTraceLog(MIC_LOG_INFO, "Waiting for %d files (timeout: %d ms)", count, timeoutMs);
    
    long startTime = micGetTimeStamp();
    int pollInterval = 100;  // Poll every 100ms
    
    while (true)
    {
        // Check all files
        bool allExist = true;
        for (int i = 0; i < count; i++)
        {
            if (!micIsFileAvailable(fileNames[i]))
            {
                allExist = false;
                break;
            }
        }
        
        if (allExist)
        {
            micTraceLog(MIC_LOG_INFO, "All %d files found", count);
            return true;
        }
        
        // Check timeout
        long currentTime = micGetTimeStamp();
        long elapsed = (currentTime - startTime) * 1000;  // Convert to ms
        
        if (elapsed >= timeoutMs)
        {
            micTraceLog(MIC_LOG_WARNING, "Timeout waiting for files");
            return false;
        }
        
        // Wait before next poll
        micWaitTime(pollInterval);
    }
}

// Check if all dependency files exist
bool micCheckDependencies(const char **fileNames, int count)
{
    if (fileNames == NULL || count <= 0) return false;
    
    bool allExist = true;
    
    for (int i = 0; i < count; i++)
    {
        if (!micIsFileAvailable(fileNames[i]))
        {
            micTraceLog(MIC_LOG_ERROR, "Dependency missing: %s", fileNames[i]);
            allExist = false;
        }
    }
    
    if (allExist)
    {
        micTraceLog(MIC_LOG_INFO, "All %d dependencies satisfied", count);
    }
    
    return allExist;
}

// HPC Features - SLURM Integration
//----------------------------------------------------------------------------------

// Submit job to SLURM scheduler
int micSlurmSubmitJob(const char *scriptFile, const char *jobName, const char *options)
{
    if (scriptFile == NULL || !micIsFileAvailable(scriptFile))
    {
        micTraceLog(MIC_LOG_ERROR, "SLURM script file not found: %s", scriptFile);
        return -1;
    }
    
    // Build sbatch command
    char command[2048];
    if (jobName != NULL && options != NULL)
    {
        snprintf(command, sizeof(command), "sbatch --job-name=%s %s %s", jobName, options, scriptFile);
    }
    else if (jobName != NULL)
    {
        snprintf(command, sizeof(command), "sbatch --job-name=%s %s", jobName, scriptFile);
    }
    else if (options != NULL)
    {
        snprintf(command, sizeof(command), "sbatch %s %s", options, scriptFile);
    }
    else
    {
        snprintf(command, sizeof(command), "sbatch %s", scriptFile);
    }
    
    micTraceLog(MIC_LOG_INFO, "Submitting SLURM job: %s", command);
    
    // Execute sbatch and capture output
    FILE *fp = popen(command, "r");
    if (fp == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Failed to execute sbatch command");
        return -1;
    }
    
    char output[256];
    int jobId = -1;
    
    while (fgets(output, sizeof(output), fp) != NULL)
    {
        // Parse job ID from "Submitted batch job 12345"
        if (strstr(output, "Submitted batch job") != NULL)
        {
            char *ptr = strstr(output, "job") + 4;
            while (*ptr == ' ') ptr++;
            jobId = atoi(ptr);
            break;
        }
    }
    
    pclose(fp);
    
    if (jobId > 0)
    {
        micTraceLog(MIC_LOG_INFO, "SLURM job submitted: ID=%d", jobId);
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "Failed to parse SLURM job ID");
    }
    
    return jobId;
}

// Get SLURM job status
int micSlurmJobStatus(int jobId)
{
    if (jobId <= 0) return -1;
    
    char command[256];
    snprintf(command, sizeof(command), "squeue -j %d -h -o %%T", jobId);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) return -1;
    
    char status[64];
    int result = -1;
    
    if (fgets(status, sizeof(status), fp) != NULL)
    {
        // Parse status: PENDING, RUNNING, COMPLETED, FAILED, CANCELLED
        if (strstr(status, "RUNNING") || strstr(status, "PENDING"))
        {
            result = 0;  // Running
        }
        else if (strstr(status, "COMPLETED"))
        {
            result = 1;  // Completed
        }
        else if (strstr(status, "FAILED") || strstr(status, "CANCELLED") || strstr(status, "TIMEOUT"))
        {
            result = 2;  // Failed
        }
    }
    else
    {
        // Job not found in queue (completed or failed)
        // Check sacct for completed jobs
        pclose(fp);
        snprintf(command, sizeof(command), "sacct -j %d -n -o State -X", jobId);
        fp = popen(command, "r");
        if (fp != NULL && fgets(status, sizeof(status), fp) != NULL)
        {
            if (strstr(status, "COMPLETED"))
            {
                result = 1;
            }
            else
            {
                result = 2;
            }
        }
    }
    
    pclose(fp);
    return result;
}

// Wait for SLURM job completion
bool micSlurmWaitForJob(int jobId, int timeoutSec)
{
    if (jobId <= 0) return false;
    
    micTraceLog(MIC_LOG_INFO, "Waiting for SLURM job %d (timeout: %d sec)", jobId, timeoutSec);
    
    long startTime = micGetTimeStamp();
    
    while (true)
    {
        int status = micSlurmJobStatus(jobId);
        
        if (status == 1)
        {
            micTraceLog(MIC_LOG_INFO, "SLURM job %d completed successfully", jobId);
            return true;
        }
        else if (status == 2)
        {
            micTraceLog(MIC_LOG_ERROR, "SLURM job %d failed", jobId);
            return false;
        }
        else if (status == -1)
        {
            // Job not found, assume completed
            return true;
        }
        
        // Check timeout
        long currentTime = micGetTimeStamp();
        if (currentTime - startTime >= timeoutSec)
        {
            micTraceLog(MIC_LOG_WARNING, "Timeout waiting for SLURM job %d", jobId);
            return false;
        }
        
        // Wait 5 seconds before next check
        micWaitTime(5000);
    }
}

// Cancel SLURM job
bool micSlurmCancelJob(int jobId)
{
    if (jobId <= 0) return false;
    
    char command[256];
    snprintf(command, sizeof(command), "scancel %d", jobId);
    
    int result = system(command);
    
    if (result == 0)
    {
        micTraceLog(MIC_LOG_INFO, "SLURM job %d cancelled", jobId);
        return true;
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "Failed to cancel SLURM job %d", jobId);
        return false;
    }
}

// Get SLURM job output
char* micSlurmGetJobOutput(int jobId, const char *outputFile)
{
    if (outputFile == NULL || !micIsFileAvailable(outputFile))
    {
        micTraceLog(MIC_LOG_WARNING, "SLURM output file not found: %s", outputFile);
        return NULL;
    }
    
    return micLoadFileText(outputFile);
}

// HPC Features - Container Support
//----------------------------------------------------------------------------------

// Execute command in Singularity container
int micSingularityExec(const char *image, const char *command, const char *bindPaths)
{
    if (image == NULL || command == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Singularity: image and command required");
        return -1;
    }
    
    char fullCommand[2048];
    
    if (bindPaths != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity exec --bind %s %s %s", bindPaths, image, command);
    }
    else
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity exec %s %s", image, command);
    }
    
    micTraceLog(MIC_LOG_INFO, "Executing Singularity: %s", fullCommand);
    
    int result = system(fullCommand);
    return WEXITSTATUS(result);
}

// Run Singularity container
int micSingularityRun(const char *image, const char *args, const char *bindPaths)
{
    if (image == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Singularity: image required");
        return -1;
    }
    
    char fullCommand[2048];
    
    if (bindPaths != NULL && args != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity run --bind %s %s %s", bindPaths, image, args);
    }
    else if (bindPaths != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity run --bind %s %s", bindPaths, image);
    }
    else if (args != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity run %s %s", image, args);
    }
    else
    {
        snprintf(fullCommand, sizeof(fullCommand), "singularity run %s", image);
    }
    
    micTraceLog(MIC_LOG_INFO, "Running Singularity: %s", fullCommand);
    
    int result = system(fullCommand);
    return WEXITSTATUS(result);
}

// Run Docker container
int micDockerRun(const char *image, const char *command, const char *volumes)
{
    if (image == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Docker: image required");
        return -1;
    }
    
    char fullCommand[2048];
    
    if (volumes != NULL && command != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "docker run --rm -v %s %s %s", volumes, image, command);
    }
    else if (volumes != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "docker run --rm -v %s %s", volumes, image);
    }
    else if (command != NULL)
    {
        snprintf(fullCommand, sizeof(fullCommand), "docker run --rm %s %s", image, command);
    }
    else
    {
        snprintf(fullCommand, sizeof(fullCommand), "docker run --rm %s", image);
    }
    
    micTraceLog(MIC_LOG_INFO, "Running Docker: %s", fullCommand);
    
    int result = system(fullCommand);
    return WEXITSTATUS(result);
}

// Generic container execution
int micContainerExec(const char *containerType, const char *image, const char *command, const char *mounts)
{
    if (containerType == NULL || image == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Container: type and image required");
        return -1;
    }
    
    if (strcmp(containerType, "singularity") == 0)
    {
        return micSingularityExec(image, command, mounts);
    }
    else if (strcmp(containerType, "docker") == 0)
    {
        return micDockerRun(image, command, mounts);
    }
    else
    {
        micTraceLog(MIC_LOG_ERROR, "Unknown container type: %s", containerType);
        return -1;
    }
}

// HPC Features - Asynchronous Execution
//----------------------------------------------------------------------------------

// Execute command asynchronously
int micAsyncExecute(const char *command)
{
    if (command == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Async: command required");
        return -1;
    }
    
    micTraceLog(MIC_LOG_INFO, "Executing async: %s", command);
    
    #if defined(_WIN32)
        // Windows implementation
        micTraceLog(MIC_LOG_WARNING, "Async execution not fully implemented on Windows");
        return -1;
    #else
        // Unix/Linux implementation
        pid_t pid = fork();
        
        if (pid < 0)
        {
            micTraceLog(MIC_LOG_ERROR, "Failed to fork process");
            return -1;
        }
        else if (pid == 0)
        {
            // Child process
            execl("/bin/sh", "sh", "-c", command, (char *)NULL);
            exit(1);  // If execl fails
        }
        else
        {
            // Parent process
            micTraceLog(MIC_LOG_INFO, "Async process started: PID=%d", pid);
            return pid;
        }
    #endif
}

// Check if async process is running
bool micAsyncIsRunning(int pid)
{
    if (pid <= 0) return false;
    
    #if defined(_WIN32)
        return false;
    #else
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        return (result == 0);  // 0 means still running
    #endif
}

// Wait for async process
int micAsyncWait(int pid, int timeoutSec)
{
    if (pid <= 0) return -1;
    
    micTraceLog(MIC_LOG_INFO, "Waiting for async process %d (timeout: %d sec)", pid, timeoutSec);
    
    #if defined(_WIN32)
        return -1;
    #else
        long startTime = micGetTimeStamp();
        
        while (true)
        {
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result == pid)
            {
                // Process finished
                if (WIFEXITED(status))
                {
                    int exitCode = WEXITSTATUS(status);
                    micTraceLog(MIC_LOG_INFO, "Async process %d exited with code %d", pid, exitCode);
                    return exitCode;
                }
                else
                {
                    micTraceLog(MIC_LOG_WARNING, "Async process %d terminated abnormally", pid);
                    return -1;
                }
            }
            else if (result < 0)
            {
                micTraceLog(MIC_LOG_ERROR, "Error waiting for process %d", pid);
                return -1;
            }
            
            // Check timeout
            long currentTime = micGetTimeStamp();
            if (currentTime - startTime >= timeoutSec)
            {
                micTraceLog(MIC_LOG_WARNING, "Timeout waiting for async process %d", pid);
                return -1;
            }
            
            // Wait before next check
            micWaitTime(100);
        }
    #endif
}

// Cancel async process
bool micAsyncCancel(int pid)
{
    if (pid <= 0) return false;
    
    #if defined(_WIN32)
        return false;
    #else
        int result = kill(pid, SIGTERM);
        
        if (result == 0)
        {
            micTraceLog(MIC_LOG_INFO, "Async process %d terminated", pid);
            return true;
        }
        else
        {
            micTraceLog(MIC_LOG_ERROR, "Failed to terminate async process %d", pid);
            return false;
        }
    #endif
}

// Get output from async process
char* micAsyncGetOutput(int pid, const char *outputFile)
{
    if (outputFile == NULL || !micIsFileAvailable(outputFile))
    {
        micTraceLog(MIC_LOG_WARNING, "Async output file not found: %s", outputFile);
        return NULL;
    }
    
    return micLoadFileText(outputFile);
}

// HPC Features - Multithreading
//----------------------------------------------------------------------------------

#if !defined(MIC_NO_THREADS)

// Thread wrapper function
#if defined(_WIN32)
static DWORD WINAPI micThreadWrapper(LPVOID arg)
{
    micThread *thread = (micThread *)arg;
    thread->function(thread->arg);
    return 0;
}
#else
static void* micThreadWrapper(void *arg)
{
    micThread *thread = (micThread *)arg;
    thread->function(thread->arg);
    return NULL;
}
#endif

// Create and start thread
micThread* micThreadCreate(micThreadFunction func, void *arg)
{
    if (func == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Thread: function required");
        return NULL;
    }
    
    micThread *thread = (micThread *)MIC_MALLOC(sizeof(micThread));
    if (thread == NULL) return NULL;
    
    thread->function = func;
    thread->arg = arg;
    thread->detached = false;
    
    #if defined(_WIN32)
        thread->handle = CreateThread(NULL, 0, micThreadWrapper, thread, 0, NULL);
        if (thread->handle == NULL)
        {
            MIC_FREE(thread);
            micTraceLog(MIC_LOG_ERROR, "Failed to create thread");
            return NULL;
        }
    #else
        int result = pthread_create(&thread->thread, NULL, micThreadWrapper, thread);
        if (result != 0)
        {
            MIC_FREE(thread);
            micTraceLog(MIC_LOG_ERROR, "Failed to create thread");
            return NULL;
        }
    #endif
    
    micTraceLog(MIC_LOG_INFO, "Thread created");
    return thread;
}

// Wait for thread completion
bool micThreadJoin(micThread *thread, int timeoutMs)
{
    if (thread == NULL) return false;
    
    if (thread->detached)
    {
        micTraceLog(MIC_LOG_WARNING, "Cannot join detached thread");
        return false;
    }
    
    #if defined(_WIN32)
        DWORD wait = (timeoutMs < 0) ? INFINITE : timeoutMs;
        DWORD result = WaitForSingleObject(thread->handle, wait);
        CloseHandle(thread->handle);
        MIC_FREE(thread);
        return (result == WAIT_OBJECT_0);
    #else
        // For pthread, timeout is complex - just do infinite wait for simplicity
        // A production implementation would use condition variables or signals
        if (timeoutMs < 0 || timeoutMs > 0)
        {
            pthread_join(thread->thread, NULL);
            MIC_FREE(thread);
            return true;
        }
        return false;
    #endif
}

// Detach thread
void micThreadDetach(micThread *thread)
{
    if (thread == NULL) return;
    
    #if defined(_WIN32)
        CloseHandle(thread->handle);
    #else
        pthread_detach(thread->thread);
    #endif
    
    thread->detached = true;
    micTraceLog(MIC_LOG_INFO, "Thread detached");
}

// Create mutex
micMutex* micMutexCreate(void)
{
    micMutex *mutex = (micMutex *)MIC_MALLOC(sizeof(micMutex));
    if (mutex == NULL) return NULL;
    
    #if defined(_WIN32)
        mutex->handle = CreateMutex(NULL, FALSE, NULL);
        if (mutex->handle == NULL)
        {
            MIC_FREE(mutex);
            return NULL;
        }
    #else
        pthread_mutex_init(&mutex->mutex, NULL);
    #endif
    
    return mutex;
}

// Destroy mutex
void micMutexDestroy(micMutex *mutex)
{
    if (mutex == NULL) return;
    
    #if defined(_WIN32)
        CloseHandle(mutex->handle);
    #else
        pthread_mutex_destroy(&mutex->mutex);
    #endif
    
    MIC_FREE(mutex);
}

// Lock mutex
void micMutexLock(micMutex *mutex)
{
    if (mutex == NULL) return;
    
    #if defined(_WIN32)
        WaitForSingleObject(mutex->handle, INFINITE);
    #else
        pthread_mutex_lock(&mutex->mutex);
    #endif
}

// Unlock mutex
void micMutexUnlock(micMutex *mutex)
{
    if (mutex == NULL) return;
    
    #if defined(_WIN32)
        ReleaseMutex(mutex->handle);
    #else
        pthread_mutex_unlock(&mutex->mutex);
    #endif
}

// Get number of CPU cores
int micGetNumCores(void)
{
    #if defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    #elif defined(__linux__) || defined(__APPLE__)
        return sysconf(_SC_NPROCESSORS_ONLN);
    #else
        return 1;
    #endif
}

#endif  // MIC_NO_THREADS

//----------------------------------------------------------------------------------
// Bioinformatics Workflow Functions Implementation
//----------------------------------------------------------------------------------

// Configuration Management Functions

// Load configuration from simple key=value file
int micLoadConfig(micConfig *config, const char *fileName)
{
    if (config == NULL || fileName == NULL) return -1;
    
    config->count = 0;
    
    char *text = micLoadFileText(fileName);
    if (text == NULL)
    {
        micTraceLog(MIC_LOG_WARNING, "Failed to load config file: %s", fileName);
        return -1;
    }
    
    // Parse line by line
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    for (int i = 0; i < lineCount && config->count < MIC_MAX_CONFIG_ENTRIES; i++)
    {
        const char *line = lines[i];
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#' || line[0] == ';') continue;
        
        // Find '=' separator
        const char *eq = strchr(line, '=');
        if (eq == NULL) continue;
        
        // Extract key and value
        int keyLen = eq - line;
        if (keyLen >= 255) continue;  // Leave room for null terminator
        
        strncpy(config->entries[config->count].key, line, keyLen);
        config->entries[config->count].key[keyLen] = '\0';
        
        // Trim whitespace from key
        int start = 0, end = keyLen - 1;
        while (start < keyLen && (config->entries[config->count].key[start] == ' ' || 
               config->entries[config->count].key[start] == '\t')) start++;
        while (end > start && (config->entries[config->count].key[end] == ' ' || 
               config->entries[config->count].key[end] == '\t')) end--;
        
        if (start > 0 || end < keyLen - 1)
        {
            memmove(config->entries[config->count].key, 
                   config->entries[config->count].key + start, 
                   end - start + 1);
            config->entries[config->count].key[end - start + 1] = '\0';
        }
        
        // Copy value
        const char *value = eq + 1;
        int valueLen = strlen(value);
        if (valueLen >= 1024) valueLen = 1023;  // Limit to buffer size - 1
        
        strncpy(config->entries[config->count].value, value, valueLen);
        config->entries[config->count].value[valueLen] = '\0';
        
        // Trim whitespace from value
        int vlen = strlen(config->entries[config->count].value);
        start = 0; end = vlen - 1;
        while (start < vlen && (config->entries[config->count].value[start] == ' ' || 
               config->entries[config->count].value[start] == '\t')) start++;
        while (end > start && (config->entries[config->count].value[end] == ' ' || 
               config->entries[config->count].value[end] == '\t' ||
               config->entries[config->count].value[end] == '\r' ||
               config->entries[config->count].value[end] == '\n')) end--;
        
        if (start > 0 || end < vlen - 1)
        {
            memmove(config->entries[config->count].value, 
                   config->entries[config->count].value + start, 
                   end - start + 1);
            config->entries[config->count].value[end - start + 1] = '\0';
        }
        
        config->count++;
    }
    
    micUnloadFileText(text);
    
    micTraceLog(MIC_LOG_INFO, "Loaded %d config entries from %s", config->count, fileName);
    return config->count;
}

// Load configuration from JSON file (simple parser)
int micLoadConfigJSON(micConfig *config, const char *fileName)
{
    if (config == NULL || fileName == NULL) return -1;
    
    config->count = 0;
    
    char *text = micLoadFileText(fileName);
    if (text == NULL)
    {
        micTraceLog(MIC_LOG_WARNING, "Failed to load JSON config file: %s", fileName);
        return -1;
    }
    
    // Simple JSON parsing - look for "key": "value" pairs
    char *ptr = text;
    while (*ptr && config->count < MIC_MAX_CONFIG_ENTRIES)
    {
        // Find opening quote for key
        char *keyStart = strchr(ptr, '"');
        if (keyStart == NULL) break;
        keyStart++;
        
        // Find closing quote for key
        char *keyEnd = strchr(keyStart, '"');
        if (keyEnd == NULL) break;
        
        // Extract key
        int keyLen = keyEnd - keyStart;
        if (keyLen >= 255) { ptr = keyEnd + 1; continue; }  // Leave room for null terminator
        
        strncpy(config->entries[config->count].key, keyStart, keyLen);
        config->entries[config->count].key[keyLen] = '\0';
        
        // Find colon
        ptr = strchr(keyEnd, ':');
        if (ptr == NULL) break;
        ptr++;
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Find value (string or other)
        if (*ptr == '"')
        {
            // String value
            ptr++;
            char *valueStart = ptr;
            char *valueEnd = strchr(valueStart, '"');
            if (valueEnd == NULL) break;
            
            int valueLen = valueEnd - valueStart;
            if (valueLen >= 1024) valueLen = 1023;
            
            strncpy(config->entries[config->count].value, valueStart, valueLen);
            config->entries[config->count].value[valueLen] = '\0';
            
            ptr = valueEnd + 1;
        }
        else
        {
            // Number, boolean, etc
            char *valueStart = ptr;
            while (*ptr && *ptr != ',' && *ptr != '}' && *ptr != '\n') ptr++;
            
            int valueLen = ptr - valueStart;
            if (valueLen >= 1024) valueLen = 1023;
            
            strncpy(config->entries[config->count].value, valueStart, valueLen);
            config->entries[config->count].value[valueLen] = '\0';
            
            // Trim trailing whitespace
            int i = valueLen - 1;
            while (i >= 0 && (config->entries[config->count].value[i] == ' ' || 
                   config->entries[config->count].value[i] == '\t' ||
                   config->entries[config->count].value[i] == '\r')) 
            {
                config->entries[config->count].value[i] = '\0';
                i--;
            }
        }
        
        config->count++;
    }
    
    micUnloadFileText(text);
    
    micTraceLog(MIC_LOG_INFO, "Loaded %d config entries from JSON %s", config->count, fileName);
    return config->count;
}

// Get configuration value by key
const char *micConfigGet(micConfig *config, const char *key)
{
    if (config == NULL || key == NULL) return NULL;
    
    for (int i = 0; i < config->count; i++)
    {
        if (strcmp(config->entries[i].key, key) == 0)
        {
            return config->entries[i].value;
        }
    }
    
    return NULL;
}

// Get configuration value with default
const char *micConfigGetDefault(micConfig *config, const char *key, const char *defaultValue)
{
    const char *value = micConfigGet(config, key);
    return (value != NULL) ? value : defaultValue;
}

// Set configuration value
void micConfigSet(micConfig *config, const char *key, const char *value)
{
    if (config == NULL || key == NULL || value == NULL) return;
    
    // Check if key already exists
    for (int i = 0; i < config->count; i++)
    {
        if (strcmp(config->entries[i].key, key) == 0)
        {
            strncpy(config->entries[i].value, value, 1023);
            config->entries[i].value[1023] = '\0';
            return;
        }
    }
    
    // Add new entry if space available
    if (config->count < MIC_MAX_CONFIG_ENTRIES)
    {
        strncpy(config->entries[config->count].key, key, 255);
        config->entries[config->count].key[255] = '\0';
        strncpy(config->entries[config->count].value, value, 1023);
        config->entries[config->count].value[1023] = '\0';
        config->count++;
    }
}

// Free configuration memory (currently no dynamic allocation)
void micConfigFree(micConfig *config)
{
    if (config == NULL) return;
    config->count = 0;
}

// Sample List Management Functions

// Load samples from JSON array
int micLoadSamplesJSON(micSampleList *samples, const char *fileName)
{
    if (samples == NULL || fileName == NULL) return -1;
    
    samples->samples = NULL;
    samples->count = 0;
    
    char *text = micLoadFileText(fileName);
    if (text == NULL)
    {
        micTraceLog(MIC_LOG_WARNING, "Failed to load samples JSON: %s", fileName);
        return -1;
    }
    
    // Simple JSON array parser - look for strings in array
    char *ptr = text;
    
    // Find opening bracket
    ptr = strchr(ptr, '[');
    if (ptr == NULL)
    {
        micUnloadFileText(text);
        return -1;
    }
    ptr++;
    
    // Allocate sample array
    samples->samples = (char **)MIC_MALLOC(MIC_MAX_SAMPLES * sizeof(char *));
    if (samples->samples == NULL)
    {
        micUnloadFileText(text);
        return -1;
    }
    
    // Parse samples
    while (*ptr && samples->count < MIC_MAX_SAMPLES)
    {
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r' || *ptr == ',') ptr++;
        
        if (*ptr == ']') break;  // End of array
        
        // Find opening quote
        if (*ptr != '"') { ptr++; continue; }
        ptr++;
        
        char *sampleStart = ptr;
        char *sampleEnd = strchr(sampleStart, '"');
        if (sampleEnd == NULL) break;
        
        // Allocate and copy sample name
        int sampleLen = sampleEnd - sampleStart;
        samples->samples[samples->count] = (char *)MIC_MALLOC(sampleLen + 1);
        if (samples->samples[samples->count] == NULL) break;
        
        strncpy(samples->samples[samples->count], sampleStart, sampleLen);
        samples->samples[samples->count][sampleLen] = '\0';
        
        samples->count++;
        ptr = sampleEnd + 1;
    }
    
    micUnloadFileText(text);
    
    micTraceLog(MIC_LOG_INFO, "Loaded %d samples from JSON %s", samples->count, fileName);
    return samples->count;
}

// Load samples from text file (one per line)
int micLoadSamplesList(micSampleList *samples, const char *fileName)
{
    if (samples == NULL || fileName == NULL) return -1;
    
    samples->samples = NULL;
    samples->count = 0;
    
    char *text = micLoadFileText(fileName);
    if (text == NULL)
    {
        micTraceLog(MIC_LOG_WARNING, "Failed to load samples list: %s", fileName);
        return -1;
    }
    
    // Split by lines
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    if (lineCount == 0)
    {
        micUnloadFileText(text);
        return 0;
    }
    
    // Allocate sample array
    samples->samples = (char **)MIC_MALLOC(MIC_MAX_SAMPLES * sizeof(char *));
    if (samples->samples == NULL)
    {
        micUnloadFileText(text);
        return -1;
    }
    
    // Copy non-empty lines as samples
    for (int i = 0; i < lineCount && samples->count < MIC_MAX_SAMPLES; i++)
    {
        const char *line = lines[i];
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Allocate and copy sample
        int len = strlen(line);
        
        // Remove trailing whitespace
        while (len > 0 && (line[len-1] == ' ' || line[len-1] == '\t' || 
               line[len-1] == '\r' || line[len-1] == '\n'))
        {
            len--;
        }
        
        if (len == 0) continue;
        
        samples->samples[samples->count] = (char *)MIC_MALLOC(len + 1);
        if (samples->samples[samples->count] == NULL) break;
        
        strncpy(samples->samples[samples->count], line, len);
        samples->samples[samples->count][len] = '\0';
        
        samples->count++;
    }
    
    micUnloadFileText(text);
    
    micTraceLog(MIC_LOG_INFO, "Loaded %d samples from list %s", samples->count, fileName);
    return samples->count;
}

// Create sample list from array
int micCreateSampleList(micSampleList *samples, const char **sampleNames, int count)
{
    if (samples == NULL || sampleNames == NULL || count <= 0) return -1;
    
    samples->samples = NULL;
    samples->count = 0;
    
    if (count > MIC_MAX_SAMPLES) count = MIC_MAX_SAMPLES;
    
    samples->samples = (char **)MIC_MALLOC(count * sizeof(char *));
    if (samples->samples == NULL) return -1;
    
    for (int i = 0; i < count; i++)
    {
        int len = strlen(sampleNames[i]);
        samples->samples[i] = (char *)MIC_MALLOC(len + 1);
        if (samples->samples[i] == NULL)
        {
            // Free already allocated samples
            for (int j = 0; j < i; j++) MIC_FREE(samples->samples[j]);
            MIC_FREE(samples->samples);
            samples->samples = NULL;
            return -1;
        }
        strcpy(samples->samples[i], sampleNames[i]);
        samples->count++;
    }
    
    micTraceLog(MIC_LOG_INFO, "Created sample list with %d samples", samples->count);
    return samples->count;
}

// Get sample by index
const char *micGetSample(micSampleList *samples, int index)
{
    if (samples == NULL || index < 0 || index >= samples->count) return NULL;
    return samples->samples[index];
}

// Free sample list
void micFreeSampleList(micSampleList *samples)
{
    if (samples == NULL) return;
    
    if (samples->samples != NULL)
    {
        for (int i = 0; i < samples->count; i++)
        {
            if (samples->samples[i] != NULL)
            {
                MIC_FREE(samples->samples[i]);
            }
        }
        MIC_FREE(samples->samples);
        samples->samples = NULL;
    }
    
    samples->count = 0;
}

// File Pattern Expansion Functions

// Expand pattern with sample list (e.g., "tags/{sample}.csv")
char **micExpand(const char *pattern, micSampleList *samples, int *count)
{
    if (pattern == NULL || samples == NULL || count == NULL) return NULL;
    
    *count = 0;
    
    if (samples->count == 0) return NULL;
    
    // Allocate array for expanded filenames
    char **files = (char **)MIC_MALLOC(samples->count * sizeof(char *));
    if (files == NULL) return NULL;
    
    // Expand pattern for each sample
    for (int i = 0; i < samples->count; i++)
    {
        const char *sample = samples->samples[i];
        
        // Replace {sample} with actual sample name
        char *expanded = micReplaceWildcard(pattern, "sample", sample);
        
        if (expanded != NULL)
        {
            files[*count] = expanded;
            (*count)++;
        }
    }
    
    micTraceLog(MIC_LOG_DEBUG, "Expanded pattern '%s' to %d files", pattern, *count);
    return files;
}

// Expand pattern with key-value map
char **micExpandWithMap(const char *pattern, const char **keys, const char **values, int mapSize, int *count)
{
    if (pattern == NULL || keys == NULL || values == NULL || count == NULL || mapSize <= 0) return NULL;
    
    *count = 0;
    
    // Check pattern length
    int patternLen = strlen(pattern);
    if (patternLen >= MIC_MAX_PATTERN_LENGTH) return NULL;
    
    // For single expansion with multiple replacements
    char *result = (char *)MIC_MALLOC(MIC_MAX_PATTERN_LENGTH);
    if (result == NULL) return NULL;
    
    strncpy(result, pattern, MIC_MAX_PATTERN_LENGTH - 1);
    result[MIC_MAX_PATTERN_LENGTH - 1] = '\0';
    
    // Replace each wildcard
    for (int i = 0; i < mapSize; i++)
    {
        char *temp = micReplaceWildcard(result, keys[i], values[i]);
        if (temp != NULL)
        {
            int tempLen = strlen(temp);
            if (tempLen < MIC_MAX_PATTERN_LENGTH)
            {
                strncpy(result, temp, MIC_MAX_PATTERN_LENGTH - 1);
                result[MIC_MAX_PATTERN_LENGTH - 1] = '\0';
            }
            MIC_FREE(temp);
        }
    }
    
    // Allocate result array with single element
    char **files = (char **)MIC_MALLOC(sizeof(char *));
    if (files == NULL)
    {
        MIC_FREE(result);
        return NULL;
    }
    
    files[0] = result;
    *count = 1;
    
    return files;
}

// Free expanded file list
void micFreeExpandedFiles(char **files, int count)
{
    if (files == NULL) return;
    
    for (int i = 0; i < count; i++)
    {
        if (files[i] != NULL)
        {
            MIC_FREE(files[i]);
        }
    }
    MIC_FREE(files);
}

// Check if all files exist
bool micAllFilesExist(char **files, int count)
{
    if (files == NULL || count <= 0) return false;
    
    for (int i = 0; i < count; i++)
    {
        if (!micIsFileAvailable(files[i]))
        {
            micTraceLog(MIC_LOG_DEBUG, "File does not exist: %s", files[i]);
            return false;
        }
    }
    
    return true;
}

// Wait for all files with timeout
bool micWaitForAllFiles(char **files, int count, int timeoutMs)
{
    if (files == NULL || count <= 0) return false;
    
    return micWaitForFiles((const char **)files, count, timeoutMs);
}

// Wildcard/Pattern Functions

// Extract wildcard value from filename
char *micExtractWildcard(const char *pattern, const char *fileName, const char *wildcardName)
{
    if (pattern == NULL || fileName == NULL || wildcardName == NULL) return NULL;
    
    // Build wildcard pattern: {wildcardName}
    char wildcard[128];
    snprintf(wildcard, sizeof(wildcard), "{%s}", wildcardName);
    
    // Find wildcard in pattern
    const char *wildcardPos = strstr(pattern, wildcard);
    if (wildcardPos == NULL) return NULL;
    
    // Calculate prefix and suffix lengths
    int prefixLen = wildcardPos - pattern;
    int suffixLen = strlen(pattern) - prefixLen - strlen(wildcard);
    
    // Check if filename matches prefix and suffix
    if (strncmp(fileName, pattern, prefixLen) != 0) return NULL;
    
    int fileNameLen = strlen(fileName);
    if (suffixLen > 0)
    {
        if (fileNameLen < prefixLen + suffixLen) return NULL;
        if (strcmp(fileName + fileNameLen - suffixLen, pattern + prefixLen + strlen(wildcard)) != 0)
            return NULL;
    }
    
    // Extract wildcard value
    int valueLen = fileNameLen - prefixLen - suffixLen;
    if (valueLen <= 0) return NULL;
    
    char *value = (char *)MIC_MALLOC(valueLen + 1);
    if (value == NULL) return NULL;
    
    strncpy(value, fileName + prefixLen, valueLen);
    value[valueLen] = '\0';
    
    return value;
}

// Replace wildcard in pattern
char *micReplaceWildcard(const char *pattern, const char *wildcardName, const char *value)
{
    if (pattern == NULL || wildcardName == NULL || value == NULL) return NULL;
    
    // Build wildcard pattern: {wildcardName}
    char wildcard[128];
    snprintf(wildcard, sizeof(wildcard), "{%s}", wildcardName);
    
    // Find wildcard in pattern
    const char *wildcardPos = strstr(pattern, wildcard);
    if (wildcardPos == NULL)
    {
        // No wildcard found, return copy of pattern
        char *result = (char *)MIC_MALLOC(strlen(pattern) + 1);
        if (result != NULL) strcpy(result, pattern);
        return result;
    }
    
    // Calculate lengths
    int prefixLen = wildcardPos - pattern;
    int suffixLen = strlen(pattern) - prefixLen - strlen(wildcard);
    int resultLen = prefixLen + strlen(value) + suffixLen;
    
    // Allocate result
    char *result = (char *)MIC_MALLOC(resultLen + 1);
    if (result == NULL) return NULL;
    
    // Build result
    strncpy(result, pattern, prefixLen);
    strcpy(result + prefixLen, value);
    strcpy(result + prefixLen + strlen(value), pattern + prefixLen + strlen(wildcard));
    
    return result;
}

// Get all wildcard names from pattern
char **micGetWildcards(const char *pattern, int *count)
{
    if (pattern == NULL || count == NULL) return NULL;
    
    *count = 0;
    
    // Allocate array for wildcards
    char **wildcards = (char **)MIC_MALLOC(16 * sizeof(char *));
    if (wildcards == NULL) return NULL;
    
    // Find all {wildcard} patterns
    const char *ptr = pattern;
    while ((ptr = strchr(ptr, '{')) != NULL)
    {
        ptr++;  // Skip '{'
        const char *end = strchr(ptr, '}');
        if (end == NULL) break;
        
        int len = end - ptr;
        if (len > 0 && *count < 16)
        {
            wildcards[*count] = (char *)MIC_MALLOC(len + 1);
            if (wildcards[*count] != NULL)
            {
                strncpy(wildcards[*count], ptr, len);
                wildcards[*count][len] = '\0';
                (*count)++;
            }
        }
        
        ptr = end + 1;
    }
    
    return wildcards;
}

// CSV File Operations

// Load CSV file
micCSVFile *micLoadCSV(const char *fileName, char delimiter, bool hasHeader)
{
    if (fileName == NULL) return NULL;
    
    char *text = micLoadFileText(fileName);
    if (text == NULL)
    {
        micTraceLog(MIC_LOG_WARNING, "Failed to load CSV file: %s", fileName);
        return NULL;
    }
    
    // Allocate CSV structure
    micCSVFile *csv = (micCSVFile *)MIC_CALLOC(1, sizeof(micCSVFile));
    if (csv == NULL)
    {
        micUnloadFileText(text);
        return NULL;
    }
    
    csv->delimiter = delimiter;
    
    // Split into lines
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    if (lineCount == 0)
    {
        micUnloadFileText(text);
        MIC_FREE(csv);
        return NULL;
    }
    
    int startRow = 0;
    
    // Parse header if present
    if (hasHeader && lineCount > 0)
    {
        int fieldCount = 0;
        const char **fields = micStringSplit(lines[0], delimiter, &fieldCount);
        
        csv->headerCount = fieldCount;
        csv->headers = (char **)MIC_MALLOC(fieldCount * sizeof(char *));
        
        if (csv->headers != NULL)
        {
            for (int i = 0; i < fieldCount; i++)
            {
                int len = strlen(fields[i]);
                
                // Trim whitespace and quotes
                const char *start = fields[i];
                const char *end = fields[i] + len - 1;
                
                while (*start == ' ' || *start == '\t' || *start == '"') start++;
                while (end > start && (*end == ' ' || *end == '\t' || *end == '"' || 
                       *end == '\r' || *end == '\n')) end--;
                
                int trimLen = end - start + 1;
                csv->headers[i] = (char *)MIC_MALLOC(trimLen + 1);
                if (csv->headers[i] != NULL)
                {
                    strncpy(csv->headers[i], start, trimLen);
                    csv->headers[i][trimLen] = '\0';
                }
            }
        }
        
        startRow = 1;
    }
    
    // Parse data rows
    int maxRows = lineCount - startRow;
    if (maxRows > MIC_MAX_CSV_ROWS) maxRows = MIC_MAX_CSV_ROWS;
    
    csv->rows = (micCSVRow *)MIC_MALLOC(maxRows * sizeof(micCSVRow));
    if (csv->rows == NULL)
    {
        micUnloadFileText(text);
        micFreeCSV(csv);
        return NULL;
    }
    
    for (int i = startRow; i < lineCount && csv->rowCount < maxRows; i++)
    {
        const char *line = lines[i];
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        int fieldCount = 0;
        const char **fields = micStringSplit(line, delimiter, &fieldCount);
        
        if (fieldCount > 0)
        {
            csv->rows[csv->rowCount].fieldCount = fieldCount;
            csv->rows[csv->rowCount].fields = (char **)MIC_MALLOC(fieldCount * sizeof(char *));
            
            if (csv->rows[csv->rowCount].fields != NULL)
            {
                for (int j = 0; j < fieldCount; j++)
                {
                    int len = strlen(fields[j]);
                    
                    // Trim whitespace and quotes
                    const char *start = fields[j];
                    const char *end = fields[j] + len;
                    
                    // Move start forward past whitespace and quotes
                    while (start < end && (*start == ' ' || *start == '\t' || *start == '"')) start++;
                    
                    // Move end backward to last non-whitespace character
                    end--;  // Move to last character
                    while (end >= start && (*end == ' ' || *end == '\t' || *end == '"' || 
                           *end == '\r' || *end == '\n')) end--;
                    
                    int trimLen = (end >= start) ? (end - start + 1) : 0;
                    
                    csv->rows[csv->rowCount].fields[j] = (char *)MIC_MALLOC(trimLen + 1);
                    if (csv->rows[csv->rowCount].fields[j] != NULL)
                    {
                        if (trimLen > 0)
                        {
                            strncpy(csv->rows[csv->rowCount].fields[j], start, trimLen);
                        }
                        csv->rows[csv->rowCount].fields[j][trimLen] = '\0';
                    }
                }
                csv->rowCount++;
            }
        }
    }
    
    micUnloadFileText(text);
    
    micTraceLog(MIC_LOG_INFO, "Loaded CSV file %s: %d rows, %d columns", 
                fileName, csv->rowCount, csv->headerCount);
    
    return csv;
}

// Save CSV file
bool micSaveCSV(const char *fileName, micCSVFile *csv)
{
    if (fileName == NULL || csv == NULL) return false;
    
    FILE *file = fopen(fileName, "w");
    if (file == NULL)
    {
        micTraceLog(MIC_LOG_ERROR, "Failed to open CSV file for writing: %s", fileName);
        return false;
    }
    
    // Write headers if present
    if (csv->headers != NULL && csv->headerCount > 0)
    {
        for (int i = 0; i < csv->headerCount; i++)
        {
            fprintf(file, "%s", csv->headers[i]);
            if (i < csv->headerCount - 1) fprintf(file, "%c", csv->delimiter);
        }
        fprintf(file, "\n");
    }
    
    // Write rows
    for (int i = 0; i < csv->rowCount; i++)
    {
        for (int j = 0; j < csv->rows[i].fieldCount; j++)
        {
            fprintf(file, "%s", csv->rows[i].fields[j]);
            if (j < csv->rows[i].fieldCount - 1) fprintf(file, "%c", csv->delimiter);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    
    micTraceLog(MIC_LOG_INFO, "Saved CSV file: %s", fileName);
    return true;
}

// Get CSV value by row and column indices
const char *micCSVGetValue(micCSVFile *csv, int row, int col)
{
    if (csv == NULL || row < 0 || row >= csv->rowCount) return NULL;
    if (col < 0 || col >= csv->rows[row].fieldCount) return NULL;
    
    return csv->rows[row].fields[col];
}

// Get CSV value by row index and header name
const char *micCSVGetValueByName(micCSVFile *csv, int row, const char *headerName)
{
    if (csv == NULL || headerName == NULL || row < 0 || row >= csv->rowCount) return NULL;
    
    // Find column index by header name
    int col = -1;
    for (int i = 0; i < csv->headerCount; i++)
    {
        if (strcmp(csv->headers[i], headerName) == 0)
        {
            col = i;
            break;
        }
    }
    
    if (col < 0) return NULL;
    
    return micCSVGetValue(csv, row, col);
}

// Add row to CSV
bool micCSVAddRow(micCSVFile *csv, const char **fields, int fieldCount)
{
    if (csv == NULL || fields == NULL || fieldCount <= 0) return false;
    if (csv->rowCount >= MIC_MAX_CSV_ROWS) return false;
    
    // Reallocate rows if needed
    if (csv->rows == NULL)
    {
        csv->rows = (micCSVRow *)MIC_MALLOC(MIC_MAX_CSV_ROWS * sizeof(micCSVRow));
        if (csv->rows == NULL) return false;
    }
    
    csv->rows[csv->rowCount].fieldCount = fieldCount;
    csv->rows[csv->rowCount].fields = (char **)MIC_MALLOC(fieldCount * sizeof(char *));
    
    if (csv->rows[csv->rowCount].fields == NULL) return false;
    
    for (int i = 0; i < fieldCount; i++)
    {
        int len = strlen(fields[i]);
        csv->rows[csv->rowCount].fields[i] = (char *)MIC_MALLOC(len + 1);
        if (csv->rows[csv->rowCount].fields[i] != NULL)
        {
            strcpy(csv->rows[csv->rowCount].fields[i], fields[i]);
        }
    }
    
    csv->rowCount++;
    return true;
}

// Create empty CSV with headers
micCSVFile *micCSVCreate(const char **headers, int headerCount, char delimiter)
{
    micCSVFile *csv = (micCSVFile *)MIC_CALLOC(1, sizeof(micCSVFile));
    if (csv == NULL) return NULL;
    
    csv->delimiter = delimiter;
    csv->headerCount = headerCount;
    
    if (headerCount > 0 && headers != NULL)
    {
        csv->headers = (char **)MIC_MALLOC(headerCount * sizeof(char *));
        if (csv->headers != NULL)
        {
            for (int i = 0; i < headerCount; i++)
            {
                int len = strlen(headers[i]);
                csv->headers[i] = (char *)MIC_MALLOC(len + 1);
                if (csv->headers[i] != NULL)
                {
                    strcpy(csv->headers[i], headers[i]);
                }
            }
        }
    }
    
    return csv;
}

// Free CSV file
void micFreeCSV(micCSVFile *csv)
{
    if (csv == NULL) return;
    
    // Free headers
    if (csv->headers != NULL)
    {
        for (int i = 0; i < csv->headerCount; i++)
        {
            if (csv->headers[i] != NULL) MIC_FREE(csv->headers[i]);
        }
        MIC_FREE(csv->headers);
    }
    
    // Free rows
    if (csv->rows != NULL)
    {
        for (int i = 0; i < csv->rowCount; i++)
        {
            if (csv->rows[i].fields != NULL)
            {
                for (int j = 0; j < csv->rows[i].fieldCount; j++)
                {
                    if (csv->rows[i].fields[j] != NULL) MIC_FREE(csv->rows[i].fields[j]);
                }
                MIC_FREE(csv->rows[i].fields);
            }
        }
        MIC_FREE(csv->rows);
    }
    
    MIC_FREE(csv);
}

// Sum column values
double micCSVSumColumn(micCSVFile *csv, int colIndex)
{
    if (csv == NULL || colIndex < 0) return 0.0;
    
    double sum = 0.0;
    for (int i = 0; i < csv->rowCount; i++)
    {
        if (colIndex < csv->rows[i].fieldCount)
        {
            sum += atof(csv->rows[i].fields[colIndex]);
        }
    }
    
    return sum;
}

// Count rows where column matches value
int micCSVCountWhere(micCSVFile *csv, int colIndex, const char *value)
{
    if (csv == NULL || value == NULL || colIndex < 0) return 0;
    
    int count = 0;
    for (int i = 0; i < csv->rowCount; i++)
    {
        if (colIndex < csv->rows[i].fieldCount)
        {
            if (strcmp(csv->rows[i].fields[colIndex], value) == 0)
            {
                count++;
            }
        }
    }
    
    return count;
}

// Gzip/Compressed File Operations
// Note: These require zlib. Provide simple implementations that check for zlib availability

#if defined(SUPPORT_COMPRESSION_API)

// Load gzipped file to memory
unsigned char *micLoadGzipFile(const char *fileName, unsigned int *bytesRead)
{
    if (fileName == NULL || bytesRead == NULL) return NULL;
    
    // Use existing decompression if available
    unsigned char *compData = micLoadFileData(fileName, bytesRead);
    if (compData == NULL) return NULL;
    
    int dataLength = 0;
    unsigned char *data = micDecompressData(compData, *bytesRead, &dataLength);
    
    micUnloadFileData(compData);
    
    if (data != NULL) *bytesRead = dataLength;
    
    return data;
}

// Load gzipped text file
char *micLoadGzipFileText(const char *fileName)
{
    unsigned int bytesRead = 0;
    unsigned char *data = micLoadGzipFile(fileName, &bytesRead);
    
    if (data == NULL) return NULL;
    
    // Ensure null termination
    char *text = (char *)MIC_REALLOC(data, bytesRead + 1);
    if (text != NULL)
    {
        text[bytesRead] = '\0';
    }
    
    return text;
}

// Save data to gzipped file
bool micSaveGzipFile(const char *fileName, void *data, unsigned int size)
{
    if (fileName == NULL || data == NULL || size == 0) return false;
    
    int compSize = 0;
    unsigned char *compData = micCompressData((unsigned char *)data, size, &compSize);
    
    if (compData == NULL) return false;
    
    bool result = micSaveFileData(fileName, compData, compSize);
    
    MIC_FREE(compData);
    
    return result;
}

// Save text to gzipped file
bool micSaveGzipFileText(const char *fileName, const char *text)
{
    if (fileName == NULL || text == NULL) return false;
    
    return micSaveGzipFile(fileName, (void *)text, strlen(text));
}

// Concatenate gzipped files
bool micConcatGzipFiles(const char **srcFiles, int srcCount, const char *dstFile)
{
    if (srcFiles == NULL || srcCount <= 0 || dstFile == NULL) return false;
    
    // Load and decompress all files
    unsigned char **dataBlocks = (unsigned char **)MIC_MALLOC(srcCount * sizeof(unsigned char *));
    unsigned int *sizes = (unsigned int *)MIC_MALLOC(srcCount * sizeof(unsigned int));
    
    if (dataBlocks == NULL || sizes == NULL)
    {
        if (dataBlocks) MIC_FREE(dataBlocks);
        if (sizes) MIC_FREE(sizes);
        return false;
    }
    
    unsigned int totalSize = 0;
    for (int i = 0; i < srcCount; i++)
    {
        dataBlocks[i] = micLoadGzipFile(srcFiles[i], &sizes[i]);
        if (dataBlocks[i] != NULL)
        {
            totalSize += sizes[i];
        }
    }
    
    // Concatenate
    unsigned char *combined = (unsigned char *)MIC_MALLOC(totalSize);
    if (combined == NULL)
    {
        for (int i = 0; i < srcCount; i++)
        {
            if (dataBlocks[i]) MIC_FREE(dataBlocks[i]);
        }
        MIC_FREE(dataBlocks);
        MIC_FREE(sizes);
        return false;
    }
    
    unsigned int offset = 0;
    for (int i = 0; i < srcCount; i++)
    {
        if (dataBlocks[i] != NULL)
        {
            memcpy(combined + offset, dataBlocks[i], sizes[i]);
            offset += sizes[i];
            MIC_FREE(dataBlocks[i]);
        }
    }
    
    // Compress and save
    bool result = micSaveGzipFile(dstFile, combined, totalSize);
    
    MIC_FREE(combined);
    MIC_FREE(dataBlocks);
    MIC_FREE(sizes);
    
    return result;
}

#else

// Stub implementations when compression not available
unsigned char *micLoadGzipFile(const char *fileName, unsigned int *bytesRead)
{
    micTraceLog(MIC_LOG_WARNING, "Gzip support not available - compression library not linked");
    return NULL;
}

char *micLoadGzipFileText(const char *fileName)
{
    micTraceLog(MIC_LOG_WARNING, "Gzip support not available - compression library not linked");
    return NULL;
}

bool micSaveGzipFile(const char *fileName, void *data, unsigned int size)
{
    micTraceLog(MIC_LOG_WARNING, "Gzip support not available - compression library not linked");
    return false;
}

bool micSaveGzipFileText(const char *fileName, const char *text)
{
    micTraceLog(MIC_LOG_WARNING, "Gzip support not available - compression library not linked");
    return false;
}

bool micConcatGzipFiles(const char **srcFiles, int srcCount, const char *dstFile)
{
    micTraceLog(MIC_LOG_WARNING, "Gzip support not available - compression library not linked");
    return false;
}

#endif  // SUPPORT_COMPRESSION_API

// AWK-like Text Processing Functions

// Sum values in a specific field across all lines
double micTextFieldSum(const char *text, int fieldIndex, char delimiter)
{
    if (text == NULL || fieldIndex < 0) return 0.0;
    
    double sum = 0.0;
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    for (int i = 0; i < lineCount; i++)
    {
        if (strlen(lines[i]) == 0) continue;
        
        int fieldCount = 0;
        const char **fields = micStringSplit(lines[i], delimiter, &fieldCount);
        
        if (fieldIndex < fieldCount)
        {
            sum += atof(fields[fieldIndex]);
        }
    }
    
    return sum;
}

// Extract specific field from a line
char *micExtractField(const char *line, int fieldIndex, char delimiter)
{
    if (line == NULL || fieldIndex < 0) return NULL;
    
    int fieldCount = 0;
    const char **fields = micStringSplit(line, delimiter, &fieldCount);
    
    if (fieldIndex >= fieldCount) return NULL;
    
    // Allocate and copy field
    int len = strlen(fields[fieldIndex]);
    char *result = (char *)MIC_MALLOC(len + 1);
    if (result != NULL)
    {
        strcpy(result, fields[fieldIndex]);
    }
    
    return result;
}

// Filter lines based on field condition
char *micFilterLines(const char *text, int fieldIndex, const char *condition, char delimiter)
{
    if (text == NULL || condition == NULL || fieldIndex < 0) return NULL;
    
    // Allocate result buffer
    int textLen = strlen(text);
    char *result = (char *)MIC_MALLOC(textLen + 1);
    if (result == NULL) return NULL;
    
    result[0] = '\0';
    int resultLen = 0;
    
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    for (int i = 0; i < lineCount; i++)
    {
        if (strlen(lines[i]) == 0) continue;
        
        int fieldCount = 0;
        const char **fields = micStringSplit(lines[i], delimiter, &fieldCount);
        
        if (fieldIndex < fieldCount)
        {
            // Simple condition check: equals
            if (strcmp(fields[fieldIndex], condition) == 0)
            {
                int lineLen = strlen(lines[i]);
                if (resultLen + lineLen + 2 <= textLen)
                {
                    strcpy(result + resultLen, lines[i]);
                    resultLen += lineLen;
                    result[resultLen++] = '\n';
                    result[resultLen] = '\0';
                }
            }
        }
    }
    
    return result;
}

// Count lines in text
int micCountLines(const char *text)
{
    if (text == NULL) return 0;
    
    int count = 0;
    const char *ptr = text;
    
    while (*ptr)
    {
        if (*ptr == '\n') count++;
        ptr++;
    }
    
    // Count last line if it doesn't end with newline
    if (ptr > text && *(ptr - 1) != '\n') count++;
    
    return count;
}

// Get first n lines
char *micHeadLines(const char *text, int n)
{
    if (text == NULL || n <= 0) return NULL;
    
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    if (n > lineCount) n = lineCount;
    
    // Calculate total length needed
    int totalLen = 0;
    for (int i = 0; i < n; i++)
    {
        totalLen += strlen(lines[i]) + 1;  // +1 for newline
    }
    
    char *result = (char *)MIC_MALLOC(totalLen + 1);
    if (result == NULL) return NULL;
    
    result[0] = '\0';
    int offset = 0;
    
    for (int i = 0; i < n; i++)
    {
        int len = strlen(lines[i]);
        strcpy(result + offset, lines[i]);
        offset += len;
        result[offset++] = '\n';
    }
    result[offset] = '\0';
    
    return result;
}

// Get last n lines
char *micTailLines(const char *text, int n)
{
    if (text == NULL || n <= 0) return NULL;
    
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    int start = (lineCount > n) ? (lineCount - n) : 0;
    int count = lineCount - start;
    
    // Calculate total length needed
    int totalLen = 0;
    for (int i = start; i < lineCount; i++)
    {
        totalLen += strlen(lines[i]) + 1;  // +1 for newline
    }
    
    char *result = (char *)MIC_MALLOC(totalLen + 1);
    if (result == NULL) return NULL;
    
    result[0] = '\0';
    int offset = 0;
    
    for (int i = start; i < lineCount; i++)
    {
        int len = strlen(lines[i]);
        strcpy(result + offset, lines[i]);
        offset += len;
        result[offset++] = '\n';
    }
    result[offset] = '\0';
    
    return result;
}

// Sort lines by field
// WARNING: This uses bubble sort with O(n²) complexity. 
// For large datasets (>1000 lines), this may be slow.
// Consider limiting line count or using qsort for production use.
char *micSortByField(const char *text, int fieldIndex, char delimiter, bool descending, bool numeric)
{
    if (text == NULL || fieldIndex < 0) return NULL;
    
    int lineCount = 0;
    const char **lines = micStringSplit(text, '\n', &lineCount);
    
    if (lineCount == 0) return NULL;
    
    // Limit sorting to reasonable size for bubble sort
    if (lineCount > 10000)
    {
        micTraceLog(MIC_LOG_WARNING, "Sorting %d lines may be slow with bubble sort", lineCount);
    }
    
    // Simple bubble sort (acceptable for small-medium datasets)
    // For production with large datasets, replace with qsort and custom comparator
    const char **sortedLines = (const char **)MIC_MALLOC(lineCount * sizeof(char *));
    if (sortedLines == NULL) return NULL;
    
    for (int i = 0; i < lineCount; i++)
    {
        sortedLines[i] = lines[i];
    }
    
    // Bubble sort
    for (int i = 0; i < lineCount - 1; i++)
    {
        for (int j = 0; j < lineCount - i - 1; j++)
        {
            // Extract fields
            char *field1 = micExtractField(sortedLines[j], fieldIndex, delimiter);
            char *field2 = micExtractField(sortedLines[j + 1], fieldIndex, delimiter);
            
            if (field1 == NULL || field2 == NULL)
            {
                if (field1) MIC_FREE(field1);
                if (field2) MIC_FREE(field2);
                continue;
            }
            
            bool swap = false;
            if (numeric)
            {
                double val1 = atof(field1);
                double val2 = atof(field2);
                swap = descending ? (val1 < val2) : (val1 > val2);
            }
            else
            {
                int cmp = strcmp(field1, field2);
                swap = descending ? (cmp < 0) : (cmp > 0);
            }
            
            if (swap)
            {
                const char *temp = sortedLines[j];
                sortedLines[j] = sortedLines[j + 1];
                sortedLines[j + 1] = temp;
            }
            
            MIC_FREE(field1);
            MIC_FREE(field2);
        }
    }
    
    // Build result string
    int totalLen = 0;
    for (int i = 0; i < lineCount; i++)
    {
        totalLen += strlen(sortedLines[i]) + 1;
    }
    
    char *result = (char *)MIC_MALLOC(totalLen + 1);
    if (result != NULL)
    {
        result[0] = '\0';
        int offset = 0;
        
        for (int i = 0; i < lineCount; i++)
        {
            int len = strlen(sortedLines[i]);
            strcpy(result + offset, sortedLines[i]);
            offset += len;
            result[offset++] = '\n';
        }
        result[offset] = '\0';
    }
    
    MIC_FREE(sortedLines);
    
    return result;
}

// Workflow Context Functions

// Initialize workflow
void micWorkflowInit(micWorkflow *wf)
{
    if (wf == NULL) return;
    
    wf->config.count = 0;
    wf->samples.samples = NULL;
    wf->samples.count = 0;
    wf->workDir[0] = '\0';
    wf->expandedFiles = NULL;
    wf->expandedCount = 0;
    
    micTraceLog(MIC_LOG_INFO, "Workflow initialized");
}

// Set working directory
void micWorkflowSetWorkDir(micWorkflow *wf, const char *dirPath)
{
    if (wf == NULL || dirPath == NULL) return;
    
    strncpy(wf->workDir, dirPath, MAX_FILEPATH_LENGTH - 1);
    wf->workDir[MAX_FILEPATH_LENGTH - 1] = '\0';
    
    micTraceLog(MIC_LOG_INFO, "Workflow work directory: %s", wf->workDir);
}

// Load configuration
int micWorkflowLoadConfig(micWorkflow *wf, const char *configFile)
{
    if (wf == NULL || configFile == NULL) return -1;
    
    // Detect file type by extension
    if (micIsFileExtension(configFile, ".json"))
    {
        return micLoadConfigJSON(&wf->config, configFile);
    }
    else
    {
        return micLoadConfig(&wf->config, configFile);
    }
}

// Load samples
int micWorkflowLoadSamples(micWorkflow *wf, const char *samplesFile)
{
    if (wf == NULL || samplesFile == NULL) return -1;
    
    // Detect file type by extension
    if (micIsFileExtension(samplesFile, ".json"))
    {
        return micLoadSamplesJSON(&wf->samples, samplesFile);
    }
    else
    {
        return micLoadSamplesList(&wf->samples, samplesFile);
    }
}

// Expand pattern
char **micWorkflowExpand(micWorkflow *wf, const char *pattern, int *count)
{
    if (wf == NULL || pattern == NULL || count == NULL) return NULL;
    
    // Free previous expansion if any
    if (wf->expandedFiles != NULL)
    {
        micFreeExpandedFiles(wf->expandedFiles, wf->expandedCount);
        wf->expandedFiles = NULL;
        wf->expandedCount = 0;
    }
    
    // Expand with samples
    wf->expandedFiles = micExpand(pattern, &wf->samples, &wf->expandedCount);
    *count = wf->expandedCount;
    
    return wf->expandedFiles;
}

// Get config value
const char *micWorkflowConfig(micWorkflow *wf, const char *key)
{
    if (wf == NULL || key == NULL) return NULL;
    
    return micConfigGet(&wf->config, key);
}

// Free workflow resources
void micWorkflowFree(micWorkflow *wf)
{
    if (wf == NULL) return;
    
    micConfigFree(&wf->config);
    micFreeSampleList(&wf->samples);
    
    if (wf->expandedFiles != NULL)
    {
        micFreeExpandedFiles(wf->expandedFiles, wf->expandedCount);
        wf->expandedFiles = NULL;
        wf->expandedCount = 0;
    }
    
    micTraceLog(MIC_LOG_INFO, "Workflow resources freed");
}

// Helper Path Functions

// Build container path
char *micContainerPath(const char *containerDir, const char *containerName)
{
    if (containerDir == NULL || containerName == NULL) return NULL;
    
    int len = strlen(containerDir) + strlen(containerName) + 2;  // +2 for '/' and '\0'
    char *path = (char *)MIC_MALLOC(len);
    
    if (path != NULL)
    {
        snprintf(path, len, "%s/%s", containerDir, containerName);
    }
    
    return path;
}

// Build output path
char *micBuildPath(const char *workDir, const char *relativePath)
{
    if (workDir == NULL || relativePath == NULL) return NULL;
    
    int len = strlen(workDir) + strlen(relativePath) + 2;
    char *path = (char *)MIC_MALLOC(len);
    
    if (path != NULL)
    {
        snprintf(path, len, "%s/%s", workDir, relativePath);
    }
    
    return path;
}

// Ensure output directory exists
bool micEnsureOutputDir(const char *filePath)
{
    if (filePath == NULL) return false;
    
    // Extract directory from file path
    const char *dirPath = micGetDirectoryPath(filePath);
    
    if (dirPath != NULL && strlen(dirPath) > 0)
    {
        // Check if directory exists, create if not
        if (!micIsDirectoryAvailable(dirPath))
        {
            if (micMakeDirectory(dirPath) == 0)
            {
                micTraceLog(MIC_LOG_INFO, "Created output directory: %s", dirPath);
                return true;
            }
            else
            {
                micTraceLog(MIC_LOG_WARNING, "Failed to create output directory: %s", dirPath);
                return false;
            }
        }
        return true;
    }
    
    return false;
}

#endif   // MIC_IMPLEMENTATION
