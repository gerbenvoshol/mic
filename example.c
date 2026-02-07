/*
 * MIC Library Example
 * Demonstrates the functionality of the mic (make-it-c) library
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

int main(void)
{
    // Initialize
    micSetTraceLogLevel(MIC_LOG_INFO);
    micTraceLog(MIC_LOG_INFO, "=== MIC Library Example ===");
    
    // ====================================
    // Environment Information
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Environment Information ---");
    printf("OS: %s\n", micGetEnvironmentInfo(MIC_ENV_INFO_OS));
    printf("Platform: %s\n", micGetEnvironmentInfo(MIC_ENV_INFO_PLATFORM));
    printf("Machine Name: %s\n", micGetEnvironmentInfo(MIC_ENV_INFO_MACHINE_NAME));
    
    // ====================================
    // String Operations
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- String Operations ---");
    
    const char *test = "hello WORLD";
    printf("Original: %s\n", test);
    printf("Upper: %s\n", micStringToUpper(test));
    printf("Lower: %s\n", micStringToLower(test));
    printf("Pascal: %s\n", micStringToPascal("hello_world_test"));
    
    printf("String size: %u\n", micStringSize(test));
    printf("Contains 'WORLD': %s\n", micStringContains(test, "WORLD") ? "yes" : "no");
    printf("Starts with 'hello': %s\n", micStringStartsWith(test, "hello") ? "yes" : "no");
    printf("String equal: %s\n", micStringEqual("test", "test") ? "yes" : "no");
    
    // String manipulation
    printf("Substring (0, 5): %s\n", micStringSubstring(test, 0, 5));
    printf("Find index of 'WORLD': %d\n", micStringFindIndex(test, "WORLD"));
    printf("Format test: %s\n", micStringFormat("Number: %d, String: %s", 42, "test"));
    
    // String join
    const char *words[] = {"apple", "banana", "cherry"};
    printf("Joined: %s\n", micStringJoin(words, 3, ", "));
    
    // String to integer
    printf("String to int '12345': %d\n", micStringToInteger("12345"));
    
    // ====================================
    // File System Operations
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- File System Operations ---");
    
    // Create a test file
    const char *testFile = "test_mic_file.txt";
    const char *testData = "This is a test file created by MIC library.\nLine 2\nLine 3\n";
    
    if (micSaveFileText(testFile, (char *)testData))
    {
        printf("File created successfully\n");
        
        // Check if file exists
        printf("File exists: %s\n", micIsFileAvailable(testFile) ? "yes" : "no");
        
        // Get file info
        printf("File size: %d bytes\n", micGetFileSize(testFile));
        printf("File extension: %s\n", micGetFileExtension(testFile));
        printf("File name: %s\n", micGetFileName(testFile));
        printf("File name without ext: %s\n", micGetFileNameWithoutExt(testFile));
        
        // Load file back
        char *loadedText = micLoadFileText(testFile);
        if (loadedText != NULL)
        {
            printf("Loaded text:\n%s\n", loadedText);
            micUnloadFileText(loadedText);
        }
        
        // Copy file
        micCopyFile(testFile, "test_mic_file_copy.txt");
        
        // Clean up
        micDeleteFile(testFile);
        micDeleteFile("test_mic_file_copy.txt");
    }
    
    // Directory operations
    const char *testDir = "test_mic_dir";
    if (micMakeDirectory(testDir) == 0 || micIsDirectoryAvailable(testDir))
    {
        printf("Directory created: %s\n", testDir);
        micDeleteDirectory(testDir);
    }
    
    // Working directory
    printf("Working directory: %s\n", micGetWorkingDirectory());
    
    // ====================================
    // Random Numbers
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Random Numbers ---");
    
    micSetRandomSeed(12345);
    printf("Random (1-10): %d, %d, %d\n", 
           micGetRandomValue(1, 10),
           micGetRandomValue(1, 10),
           micGetRandomValue(1, 10));
    
    // ====================================
    // Timing
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Timing ---");
    
    long timestamp = micGetTimeStamp();
    printf("Current timestamp: %ld\n", timestamp);
    printf("Timestamp string: %s\n", micGetTimeStampString(timestamp));
    
    micInitTimer();
    printf("Timer initialized\n");
    double startTime = micGetTime();
    micWaitTime(100);  // Wait 100ms
    double endTime = micGetTime();
    printf("Elapsed time: %.2f ms\n", endTime - startTime);
    
    // ====================================
    // Storage
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Storage ---");
    
    if (micSaveStorageValue(0, 42))
    {
        printf("Saved value 42 to position 0\n");
        int loaded = micLoadStorageValue(0);
        printf("Loaded value: %d\n", loaded);
    }
    
    // ====================================
    // Process/Steps (for pipeline tracking)
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Process Tracking ---");
    
    micBeginProcess("Example Pipeline", MIC_LOG_INFO);
    micBeginStep("Step 1: Initialization", MIC_LOG_INFO);
    micEndStep();
    micBeginStep("Step 2: Processing", MIC_LOG_INFO);
    micEndStep();
    micBeginStep("Step 3: Finalization", MIC_LOG_INFO);
    micEndStep();
    micEndProcess();
    
    // ====================================
    // Finish
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n=== Example Complete ===");
    
    return 0;
}
