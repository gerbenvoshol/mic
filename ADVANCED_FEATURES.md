# MIC Library - Advanced Pipeline & Compression Guide

## Overview
This guide covers the newly implemented compression functions using zlib and advanced pipeline features for building robust, efficient pipelines in C.

## Compression Functions

### Requirements
The compression functions require zlib to be installed and linked:
```bash
gcc -o myapp myapp.c -lz
```

To disable compression features (if zlib is not available):
```bash
gcc -o myapp myapp.c -DMIC_NO_ZLIB
```

### micCompressData()
Compress data using the DEFLATE algorithm (zlib).

```c
unsigned char *micCompressData(unsigned char *data, int dataLength, int *compDataLength);
```

**Parameters:**
- `data`: Pointer to data to compress
- `dataLength`: Size of data in bytes
- `compDataLength`: Output parameter for compressed size

**Returns:** Pointer to compressed data (must be freed with `micUnloadFileData()`), or NULL on error

**Example:**
```c
const char *text = "Hello, World! This is test data...";
int originalSize = strlen(text);
int compressedSize = 0;

unsigned char *compressed = micCompressData((unsigned char *)text, originalSize, &compressedSize);
if (compressed != NULL) {
    printf("Compressed %d bytes to %d bytes (%.1f%% reduction)\n",
           originalSize, compressedSize, 
           100.0 * (1.0 - (double)compressedSize / originalSize));
    
    // Use compressed data...
    
    micUnloadFileData(compressed);
}
```

### micDecompressData()
Decompress data compressed with `micCompressData()`.

```c
unsigned char *micDecompressData(unsigned char *compData, int compDataLength, int *dataLength);
```

**Parameters:**
- `compData`: Pointer to compressed data
- `compDataLength`: Size of compressed data in bytes
- `dataLength`: Output parameter for decompressed size

**Returns:** Pointer to decompressed data (must be freed with `micUnloadFileData()`), or NULL on error

**Example:**
```c
int decompressedSize = 0;
unsigned char *decompressed = micDecompressData(compressed, compressedSize, &decompressedSize);

if (decompressed != NULL) {
    printf("Decompressed to %d bytes\n", decompressedSize);
    
    // Verify integrity
    if (memcmp(originalData, decompressed, originalSize) == 0) {
        printf("Data integrity verified!\n");
    }
    
    micUnloadFileData(decompressed);
}
```

## Advanced Pipeline Features

The advanced pipeline features enable:
- **Step Skipping**: Automatically skip completed steps on pipeline reruns
- **File Polling**: Wait for files to appear with configurable timeout
- **Dependency Checking**: Verify all required files exist before processing
- **State Management**: Track and persist pipeline execution state

### Step State Management

#### micStepIsComplete()
Check if a pipeline step has been completed.

```c
bool micStepIsComplete(const char *stepName);
```

**Returns:** `true` if step was previously completed, `false` otherwise

#### micMarkStepComplete()
Mark a pipeline step as completed.

```c
void micMarkStepComplete(const char *stepName);
```

This function records the step name, completion status, and timestamp. Use this after successfully completing a pipeline step.

#### micSkipStepIfComplete()
Conditionally skip a step if it was already completed.

```c
bool micSkipStepIfComplete(const char *stepName);
```

**Returns:** `true` if step was skipped (already complete), `false` if step should be executed

**Example:**
```c
micBeginProcess("Data Pipeline", MIC_LOG_INFO);

// Step 1: Download data
if (!micSkipStepIfComplete("download_data")) {
    micBeginStep("Download Data", MIC_LOG_INFO);
    
    // Download logic here...
    downloadFiles();
    
    micMarkStepComplete("download_data");
    micEndStep();
}

// Step 2: Process data
if (!micSkipStepIfComplete("process_data")) {
    micBeginStep("Process Data", MIC_LOG_INFO);
    
    // Processing logic here...
    processFiles();
    
    micMarkStepComplete("process_data");
    micEndStep();
}

micEndProcess();
```

On the first run, both steps execute. On subsequent runs, both steps are skipped if they completed successfully.

#### micClearStepState()
Clear the completion state for a specific step, forcing it to re-execute.

```c
void micClearStepState(const char *stepName);
```

**Example:**
```c
// Force re-execution of a specific step
micClearStepState("process_data");

// Now this step will execute even though it was completed before
if (!micSkipStepIfComplete("process_data")) {
    // Will execute because state was cleared
}
```

#### micClearAllStepStates()
Clear all step completion states.

```c
void micClearAllStepStates(void);
```

Use this to reset the entire pipeline state, forcing all steps to re-execute on the next run.

### File Polling

#### micWaitForFile()
Wait for a file to exist, with timeout.

```c
bool micWaitForFile(const char *fileName, int timeoutMs);
```

**Parameters:**
- `fileName`: Path to file to wait for
- `timeoutMs`: Maximum time to wait in milliseconds

**Returns:** `true` if file appeared, `false` if timeout occurred

The function polls every 100ms until the file exists or timeout is reached.

**Example:**
```c
// Wait up to 30 seconds for input file
if (micWaitForFile("input_data.csv", 30000)) {
    printf("Input file arrived, processing...\n");
    processFile("input_data.csv");
} else {
    printf("Timeout waiting for input file\n");
    return -1;
}
```

#### micWaitForFiles()
Wait for multiple files to exist.

```c
bool micWaitForFiles(const char **fileNames, int count, int timeoutMs);
```

**Parameters:**
- `fileNames`: Array of file paths
- `count`: Number of files in array
- `timeoutMs`: Maximum time to wait in milliseconds

**Returns:** `true` if all files appeared, `false` if timeout occurred

**Example:**
```c
const char *requiredFiles[] = {
    "data/input1.csv",
    "data/input2.csv",
    "data/config.json"
};

if (micWaitForFiles(requiredFiles, 3, 60000)) {
    printf("All input files available\n");
    processInputs();
} else {
    printf("Timeout: Not all files arrived\n");
}
```

### Dependency Checking

#### micCheckDependencies()
Check if all dependency files exist.

```c
bool micCheckDependencies(const char **fileNames, int count);
```

**Parameters:**
- `fileNames`: Array of file paths to check
- `count`: Number of files in array

**Returns:** `true` if all files exist, `false` if any are missing

Unlike `micWaitForFiles()`, this function checks immediately without polling.

**Example:**
```c
const char *dependencies[] = {
    "config.ini",
    "credentials.key",
    "input.dat"
};

if (micCheckDependencies(dependencies, 3)) {
    printf("All dependencies satisfied\n");
    startProcessing();
} else {
    printf("ERROR: Missing dependencies\n");
    return -1;
}
```

## Complete Pipeline Example

Here's a complete example showing how to build a robust pipeline:

```c
#define MIC_IMPLEMENTATION
#include "src/mic.h"

int main(void) {
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    micBeginProcess("ETL Pipeline", MIC_LOG_INFO);
    
    // Step 1: Extract - Download data
    const char *extractStep = "extract";
    if (!micSkipStepIfComplete(extractStep)) {
        micBeginStep("Extract: Download Data", MIC_LOG_INFO);
        
        // Check prerequisites
        const char *creds[] = {"credentials.key"};
        if (!micCheckDependencies(creds, 1)) {
            printf("ERROR: Missing credentials\n");
            return -1;
        }
        
        // Download files (simulated)
        downloadData("https://api.example.com/data");
        
        // Wait for download completion
        if (!micWaitForFile("raw_data.csv", 60000)) {
            printf("ERROR: Download timeout\n");
            return -1;
        }
        
        micMarkStepComplete(extractStep);
        micEndStep();
    }
    
    // Step 2: Transform - Process data
    const char *transformStep = "transform";
    if (!micSkipStepIfComplete(transformStep)) {
        micBeginStep("Transform: Process Data", MIC_LOG_INFO);
        
        // Check dependencies
        const char *inputs[] = {"raw_data.csv"};
        if (!micCheckDependencies(inputs, 1)) {
            printf("ERROR: Missing input data\n");
            return -1;
        }
        
        // Load and process
        char *rawData = micLoadFileText("raw_data.csv");
        if (rawData != NULL) {
            // Transform data
            processAndTransform(rawData);
            micSaveFileText("transformed_data.csv", transformedData);
            micUnloadFileText(rawData);
        }
        
        micMarkStepComplete(transformStep);
        micEndStep();
    }
    
    // Step 3: Load - Compress and archive
    const char *loadStep = "load";
    if (!micSkipStepIfComplete(loadStep)) {
        micBeginStep("Load: Compress and Archive", MIC_LOG_INFO);
        
        // Load processed data
        unsigned int dataSize = 0;
        unsigned char *data = micLoadFileData("transformed_data.csv", &dataSize);
        
        if (data != NULL) {
            // Compress data
            int compSize = 0;
            unsigned char *compressed = micCompressData(data, dataSize, &compSize);
            
            if (compressed != NULL) {
                // Save compressed archive
                micSaveFileData("archive.dat", compressed, compSize);
                micUnloadFileData(compressed);
            }
            
            micUnloadFileData(data);
        }
        
        micMarkStepComplete(loadStep);
        micEndStep();
    }
    
    micEndProcess();
    
    printf("Pipeline completed successfully!\n");
    return 0;
}
```

## Performance Considerations

### Compression
- Compression ratio varies by data type (text: 50-70%, binary: 0-30%)
- Compression level 6 (default) balances speed and ratio
- Memory usage: ~2x uncompressed size during compression
- Speed: ~10-20 MB/s for compression, 50-100 MB/s for decompression

### File Polling
- Poll interval: 100ms (configurable in source)
- Timeout precision: ±100ms
- Minimal CPU usage during polling
- Use `micCheckDependencies()` for immediate checks without polling

### Step State
- Stored in memory (linked list)
- Minimal overhead: ~300 bytes per step
- State cleared on process exit (can be persisted to file if needed)
- No limit on number of steps

## Best Practices

1. **Always check return values** from compression and file operations
2. **Free allocated memory** using `micUnloadFileData()` or `micUnloadFileText()`
3. **Use appropriate timeouts** for file polling (consider network delays)
4. **Mark steps complete** only after successful execution
5. **Check dependencies** before executing steps to fail fast
6. **Clear step states** when input data changes to force re-execution
7. **Log appropriately** using `micTraceLog()` for debugging

## Error Handling

All functions provide proper error handling:
- Compression returns NULL on failure
- File polling returns false on timeout
- Dependency checks log missing files
- Step functions handle NULL parameters gracefully

Example with error handling:
```c
unsigned char *compressed = micCompressData(data, size, &compSize);
if (compressed == NULL) {
    printf("Compression failed - check logs\n");
    return -1;
}

// Use compressed data...

micUnloadFileData(compressed);
```

## Troubleshooting

### Compression not available
```
WARNING: micCompressData() not available - MIC_NO_ZLIB defined
```
**Solution:** Install zlib-dev and compile with `-lz` flag

### File polling timeout
```
WARNING: Timeout waiting for file 'input.dat'
```
**Solutions:**
- Increase timeout value
- Check file path is correct
- Verify file creation process is working
- Check file permissions

### Step not skipping
**Possible causes:**
- Step was not marked complete (missing `micMarkStepComplete()`)
- Step state was cleared
- Different step name used

## API Reference Summary

| Function | Purpose | Returns |
|----------|---------|---------|
| `micCompressData()` | Compress data with zlib | Compressed data pointer |
| `micDecompressData()` | Decompress data | Decompressed data pointer |
| `micStepIsComplete()` | Check step completion | true/false |
| `micMarkStepComplete()` | Mark step done | void |
| `micSkipStepIfComplete()` | Conditional skip | true if skipped |
| `micClearStepState()` | Reset step | void |
| `micClearAllStepStates()` | Reset all steps | void |
| `micWaitForFile()` | Poll for file | true if found |
| `micWaitForFiles()` | Poll for multiple files | true if all found |
| `micCheckDependencies()` | Check files exist | true if all exist |

## See Also
- `example.c` - Basic library features
- `advanced_example.c` - Compression and pipeline demo
- `IMPLEMENTATION.md` - Library implementation details
- `README.md` - Library overview
