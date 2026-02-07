<img align="center" src="logo/mic_logo.png" width="594px">

# mic (make-it-c)
A simple and easy-to-use library to build pipelines in C

<br>

This library provides a set of platform-independent functions intended to be used for pipeline definition. The goal of the project is to allow defining pipelines by coding them in plain C, using all the benefits provided by the language itself (variables definition, conditional branching, loops, preprocessor...) and combining that with useful functions to deal with the file system, text files, timing, and other elements related to process automation.

## Status

✅ **Production Ready** - All core functions implemented and tested!

## Features

 - Plain C library
 - Portable: Single-file header-only
 - Multi-platform (Linux, Windows, macOS)
 - Optional zlib dependency for compression
 - 85+ utility functions for pipeline automation

## Functionality Provided

 - **Log system** - Configurable logging with levels and custom callbacks
 - **Environment** - OS detection, platform info, environment variables
 - **Process execution** - Command execution, process/step tracking
 - **Timing management** - High-resolution timers, timestamps, delays
 - **File system management** - File/directory operations, path manipulation
 - **String management** - Case conversion, search/replace, formatting, parsing
 - **File I/O** - Binary and text file reading/writing
 - **Data compression** - DEFLATE compression/decompression with zlib
 - **Advanced pipeline features**:
   - Step completion tracking and skipping
   - File polling with timeout
   - Dependency checking
   - Pipeline state management

## Quick Start

### Basic Usage

```c
#define MIC_IMPLEMENTATION
#include "src/mic.h"

int main(void) {
    micSetTraceLogLevel(MIC_LOG_INFO);
    micTraceLog(MIC_LOG_INFO, "Hello from MIC!");
    
    // File operations
    micSaveFileText("output.txt", "Pipeline data");
    
    // String operations
    const char *upper = micStringToUpper("hello world");
    printf("%s\n", upper);
    
    return 0;
}
```

Compile:
```bash
gcc -o myapp myapp.c
```

### Advanced Pipeline with Compression

```c
#define MIC_IMPLEMENTATION
#include "src/mic.h"

int main(void) {
    micBeginProcess("Data Pipeline", MIC_LOG_INFO);
    
    // Step 1: Download (skip if already done)
    if (!micSkipStepIfComplete("download")) {
        micBeginStep("Download Data", MIC_LOG_INFO);
        downloadFiles();
        micMarkStepComplete("download");
        micEndStep();
    }
    
    // Step 2: Process (with dependency check)
    if (!micSkipStepIfComplete("process")) {
        micBeginStep("Process Data", MIC_LOG_INFO);
        
        const char *deps[] = {"input.dat"};
        if (micCheckDependencies(deps, 1)) {
            processData();
            micMarkStepComplete("process");
        }
        micEndStep();
    }
    
    // Step 3: Compress output
    if (!micSkipStepIfComplete("compress")) {
        micBeginStep("Compress", MIC_LOG_INFO);
        
        unsigned int size = 0;
        unsigned char *data = micLoadFileData("output.dat", &size);
        
        int compSize = 0;
        unsigned char *comp = micCompressData(data, size, &compSize);
        micSaveFileData("output.dat.z", comp, compSize);
        
        micUnloadFileData(data);
        micUnloadFileData(comp);
        
        micMarkStepComplete("compress");
        micEndStep();
    }
    
    micEndProcess();
    return 0;
}
```

Compile with compression:
```bash
gcc -o myapp myapp.c -lz
```

## Documentation

- [EXAMPLE.md](EXAMPLE.md) - Basic usage examples
- [ADVANCED_FEATURES.md](ADVANCED_FEATURES.md) - Compression and pipeline features guide
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Implementation details and API reference

## Examples

- `example.c` - Demonstrates all basic features
- `advanced_example.c` - Shows compression and advanced pipeline features

Build and run:
```bash
gcc -o example example.c
./example

gcc -o advanced_example advanced_example.c -lz
./advanced_example
```

## Requirements

### Core Library
- C compiler (gcc, clang, msvc)
- Standard C library

### Optional
- zlib (for compression functions) - install with:
  - Ubuntu/Debian: `apt-get install zlib1g-dev`
  - macOS: `brew install zlib`
  - Windows: Download from zlib.net

To disable compression features, compile with:
```bash
gcc -o myapp myapp.c -DMIC_NO_ZLIB
```

## Platform Support

- ✅ Linux (tested)
- ✅ macOS (code maintained)
- ✅ Windows (code maintained)

## API Overview

### Pipeline Management
- `micBeginProcess()` / `micEndProcess()` - Process boundaries
- `micBeginStep()` / `micEndStep()` - Step boundaries
- `micStepIsComplete()` - Check step state
- `micMarkStepComplete()` - Mark step done
- `micSkipStepIfComplete()` - Conditional execution

### File Polling
- `micWaitForFile()` - Poll for single file
- `micWaitForFiles()` - Poll for multiple files
- `micCheckDependencies()` - Check file existence

### Compression (requires -lz)
- `micCompressData()` - DEFLATE compression
- `micDecompressData()` - DEFLATE decompression

### File System (20 functions)
- File: create, delete, copy, move, rename
- Directory: create, delete, copy, move, list
- Query: exists, size, modification time
- Path: extract name, extension, directory

### String Operations (17 functions)
- Case conversion, search/replace, format
- Join/split, substring, find
- Integer parsing

### File I/O (6 functions)
- Load/save binary data
- Load/save text files

### Others
- Logging, environment info, timing, random numbers, storage

See [IMPLEMENTATION.md](IMPLEMENTATION.md) for complete API reference.
  
## License

mic (make-it-c) is licensed under an unmodified MIT license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.
