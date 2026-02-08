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
 - Optional dependencies: zlib (compression), pthread (threading)
 - 180+ utility functions for pipeline automation

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
 - **HPC features**:
   - SLURM job submission and management
   - Container support (Singularity, Docker)
   - Asynchronous task execution
   - Multithreading with synchronization
 - **Bioinformatics workflow features** ✨ NEW:
   - Configuration file management (simple key=value and JSON)
   - Sample list management for batch processing
   - File pattern expansion with wildcards (Snakemake-like)
   - CSV file operations (read, write, query, aggregate)
   - Gzip/compressed file handling
   - AWK-like text processing functions
   - Workflow context management
   - Helper path functions
 - **Rule-based workflow system** 🎯 NEW:
   - Snakemake-like rule definitions with inputs, outputs, and shell commands
   - Workflow DAG for automatic dependency resolution
   - Benchmark tracking for execution time and resource usage
   - Multi-wildcard pattern expansion
   - Temporary file management with auto-cleanup
   - Resource specification and checking (CPU, memory, GPU, disk)
   - Rule inheritance and copying
   - Container support (Singularity/Docker)
   - Dynamic input functions with function pointers

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

### Rule-Based Workflow (Snakemake-like)

```c
#define MIC_IMPLEMENTATION
#include "src/mic.h"

int main(void) {
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // Create workflow DAG
    micWorkflowDAG *dag = micWorkflowDAGCreate();
    micWorkflowDAGSetWorkDir(dag, "/data/analysis");
    
    // Define trim rule
    micRule *trimRule = micRuleCreate("trim_reads");
    micRuleSetInput(trimRule, "raw/{sample}_R1.fastq.gz");
    micRuleSetInput(trimRule, "raw/{sample}_R2.fastq.gz");
    micRuleSetOutput(trimRule, "trimmed/{sample}_R1_trimmed.fastq.gz");
    micRuleSetOutput(trimRule, "trimmed/{sample}_R2_trimmed.fastq.gz");
    micRuleSetShell(trimRule, "fastp -i {input[0]} -I {input[1]} -o {output[0]} -O {output[1]}");
    micRuleSetThreads(trimRule, 4);
    micRuleSetBenchmark(trimRule, "benchmarks/{sample}.trim.txt");
    micWorkflowDAGAddRule(dag, trimRule);
    
    // Execute workflow
    micWorkflowDAGExecute(dag, "trimmed/sample1_R1_trimmed.fastq.gz");
    
    // Cleanup
    micWorkflowDAGFree(dag);
    
    return 0;
}
```

Compile with rule system:
```bash
gcc -o workflow workflow.c -lz -lm
```

## Documentation

- [EXAMPLE.md](EXAMPLE.md) - Basic usage examples
- [ADVANCED_FEATURES.md](ADVANCED_FEATURES.md) - Compression and pipeline features guide
- [HPC_FEATURES.md](HPC_FEATURES.md) - HPC: SLURM, containers, async, multithreading ✨ NEW
- [BIOINFORMATICS.md](BIOINFORMATICS.md) - Bioinformatics workflows and rule system 🎯 NEW
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Implementation details and API reference

## Examples

- `example.c` - Demonstrates all basic features
- `advanced_example.c` - Shows compression and advanced pipeline features
- `real_world_example.c` - Practical ETL pipeline demonstration
- `hpc_example.c` - HPC features: SLURM, containers, async, threads ✨ NEW
- `bioinformatics_example.c` - Bioinformatics workflow features ✨ NEW
- `rule_example.c` - Rule-based workflow system (Snakemake-like) 🎯 NEW

Build and run:
```bash
gcc -o example example.c
./example

gcc -o advanced_example advanced_example.c -lz
./advanced_example

gcc -o hpc_example hpc_example.c -lz -lpthread
./hpc_example

gcc -o bioinformatics_example bioinformatics_example.c -lm -lz
./bioinformatics_example

gcc -o rule_example rule_example.c -lz -lm
./rule_example
```

## Requirements

### Core Library
- C compiler (gcc, clang, msvc)
- Standard C library

### Optional
- **zlib** (for compression functions):
  - Ubuntu/Debian: `apt-get install zlib1g-dev`
  - macOS: `brew install zlib`
  - Windows: Download from zlib.net
- **pthread** (for multithreading):
  - Usually available on Linux/macOS
  - Link with `-lpthread`

Disable features with flags:
```bash
gcc -o myapp myapp.c -DMIC_NO_ZLIB          # No compression
gcc -o myapp myapp.c -DMIC_NO_THREADS       # No threading
gcc -o myapp myapp.c -DMIC_NO_ZLIB -DMIC_NO_THREADS  # Minimal build
```

## Platform Support

- ✅ Linux (tested)
- ✅ macOS (code maintained)
- ⚠️ Windows (code maintained, async incomplete)

## API Overview

### Pipeline Management
- `micBeginProcess()` / `micEndProcess()` - Process boundaries
- `micBeginStep()` / `micEndStep()` - Step boundaries
- `micStepIsComplete()` - Check step state
- `micMarkStepComplete()` - Mark step done
- `micSkipStepIfComplete()` - Conditional execution

### HPC - SLURM (5 functions) ✨ NEW
- `micSlurmSubmitJob()` - Submit job with sbatch
- `micSlurmJobStatus()` - Query job status
- `micSlurmWaitForJob()` - Wait for completion
- `micSlurmCancelJob()` - Cancel job
- `micSlurmGetJobOutput()` - Get output file

### HPC - Containers (4 functions) ✨ NEW
- `micSingularityExec()` / `micSingularityRun()` - Singularity containers
- `micDockerRun()` - Docker containers
- `micContainerExec()` - Generic container interface

### HPC - Async Execution (5 functions) ✨ NEW
- `micAsyncExecute()` - Start async process
- `micAsyncIsRunning()` - Check status
- `micAsyncWait()` - Wait for completion
- `micAsyncCancel()` - Terminate process
- `micAsyncGetOutput()` - Read output

### HPC - Multithreading (9 functions) ✨ NEW
- `micThreadCreate()` / `micThreadJoin()` - Thread lifecycle
- `micThreadDetach()` - Detach thread
- `micMutexCreate()` / `micMutexDestroy()` - Mutex lifecycle
- `micMutexLock()` / `micMutexUnlock()` - Synchronization
- `micGetNumCores()` - CPU core count

### Bioinformatics Workflows (60+ functions) ✨ NEW
- **Configuration**: Load/save config from key=value or JSON files
- **Sample Lists**: Manage sample batches for processing
- **Pattern Expansion**: Snakemake-like wildcard expansion
- **CSV Operations**: Complete CSV read/write/query/aggregate
- **Gzip Files**: Compressed file operations
- **Text Processing**: AWK-like field operations, filtering, sorting
- **Workflow Context**: High-level workflow management
- See [BIOINFORMATICS.md](BIOINFORMATICS.md) for detailed documentation

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

**Total Functions: 180+** (60 core + 2 compression + 8 pipeline + 23 HPC + 60+ bioinformatics + 27 others)

See [BIOINFORMATICS.md](BIOINFORMATICS.md), [HPC_FEATURES.md](HPC_FEATURES.md) and [IMPLEMENTATION.md](IMPLEMENTATION.md) for complete API reference.
  
## License

mic (make-it-c) is licensed under an unmodified MIT license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.
