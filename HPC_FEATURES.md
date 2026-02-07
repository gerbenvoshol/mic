# MIC Library - HPC Features Guide

## Overview
This guide covers the High-Performance Computing (HPC) features added to the mic library, including SLURM integration, container support, asynchronous execution, and multithreading.

## Table of Contents
1. [SLURM Integration](#slurm-integration)
2. [Container Support](#container-support)
3. [Asynchronous Execution](#asynchronous-execution)
4. [Multithreading](#multithreading)
5. [Complete Examples](#complete-examples)
6. [Compilation](#compilation)
7. [Best Practices](#best-practices)

---

## SLURM Integration

SLURM (Simple Linux Utility for Resource Management) is the most widely used job scheduler on HPC clusters. The mic library provides functions to submit, monitor, and manage SLURM jobs programmatically.

### Functions

#### micSlurmSubmitJob()
Submit a job to the SLURM scheduler.

```c
int micSlurmSubmitJob(const char *scriptFile, const char *jobName, const char *options);
```

**Parameters:**
- `scriptFile`: Path to SLURM batch script
- `jobName`: Name for the job (can be NULL)
- `options`: Additional sbatch options (can be NULL)

**Returns:** Job ID on success, -1 on failure

**Example:**
```c
// Create SLURM script
const char *script = 
    "#!/bin/bash\n"
    "#SBATCH --ntasks=16\n"
    "#SBATCH --time=01:00:00\n"
    "#SBATCH --mem=32G\n"
    "\n"
    "module load python\n"
    "python my_script.py\n";

micSaveFileText("job.sh", (char *)script);

// Submit with custom options
int jobId = micSlurmSubmitJob("job.sh", "data_analysis", "--partition=compute");

if (jobId > 0) {
    printf("Job submitted: ID=%d\n", jobId);
}
```

#### micSlurmJobStatus()
Check the status of a SLURM job.

```c
int micSlurmJobStatus(int jobId);
```

**Returns:**
- `0` - Job is running or pending
- `1` - Job completed successfully
- `2` - Job failed or was cancelled
- `-1` - Job not found

**Example:**
```c
int status = micSlurmJobStatus(jobId);
switch (status) {
    case 0: printf("Job is running\n"); break;
    case 1: printf("Job completed\n"); break;
    case 2: printf("Job failed\n"); break;
    default: printf("Job not found\n");
}
```

#### micSlurmWaitForJob()
Wait for a job to complete with timeout.

```c
bool micSlurmWaitForJob(int jobId, int timeoutSec);
```

**Parameters:**
- `jobId`: SLURM job ID
- `timeoutSec`: Maximum time to wait in seconds

**Returns:** `true` if job completed successfully, `false` otherwise

**Example:**
```c
// Wait up to 1 hour
if (micSlurmWaitForJob(jobId, 3600)) {
    printf("Job completed successfully\n");
    
    // Get output
    char *output = micSlurmGetJobOutput(jobId, "job.out");
    if (output != NULL) {
        printf("Output:\n%s\n", output);
        micUnloadFileText(output);
    }
} else {
    printf("Job failed or timed out\n");
}
```

#### micSlurmCancelJob()
Cancel a running or pending job.

```c
bool micSlurmCancelJob(int jobId);
```

**Example:**
```c
if (micSlurmCancelJob(jobId)) {
    printf("Job cancelled\n");
}
```

#### micSlurmGetJobOutput()
Read the output file from a completed job.

```c
char* micSlurmGetJobOutput(int jobId, const char *outputFile);
```

**Returns:** File contents (must be freed), or NULL if file not found

### Complete SLURM Workflow

```c
// 1. Create SLURM script
micSaveFileText("analysis.sh", script_content);

// 2. Submit job
int jobId = micSlurmSubmitJob("analysis.sh", "my_analysis", 
                               "--nodes=4 --time=02:00:00");

// 3. Monitor progress
while (micSlurmJobStatus(jobId) == 0) {
    printf("Job still running...\n");
    micWaitTime(60000);  // Check every minute
}

// 4. Get results
if (micSlurmJobStatus(jobId) == 1) {
    char *output = micSlurmGetJobOutput(jobId, "analysis.out");
    processResults(output);
    micUnloadFileText(output);
}
```

---

## Container Support

The mic library supports both Singularity and Docker containers, commonly used in HPC and cloud environments.

### Singularity

#### micSingularityExec()
Execute a command inside a Singularity container.

```c
int micSingularityExec(const char *image, const char *command, const char *bindPaths);
```

**Parameters:**
- `image`: Path to Singularity image (.sif file)
- `command`: Command to execute
- `bindPaths`: Bind mount paths (format: "/host:/container")

**Returns:** Exit code

**Example:**
```c
// Execute Python script in container with data mount
int result = micSingularityExec(
    "/apps/containers/python3.sif",
    "python /mnt/script.py",
    "/data:/mnt"
);

if (result == 0) {
    printf("Container execution successful\n");
}
```

#### micSingularityRun()
Run a Singularity container (executes container's runscript).

```c
int micSingularityRun(const char *image, const char *args, const char *bindPaths);
```

**Example:**
```c
// Run container with arguments
micSingularityRun("ml_pipeline.sif", "train --epochs 100", "/data:/data");
```

### Docker

#### micDockerRun()
Run a Docker container.

```c
int micDockerRun(const char *image, const char *command, const char *volumes);
```

**Parameters:**
- `image`: Docker image name or ID
- `command`: Command to execute (can be NULL for default)
- `volumes`: Volume mounts (format: "/host:/container")

**Example:**
```c
// Run analysis in Docker container
micDockerRun(
    "python:3.9",
    "python /workspace/analyze.py",
    "/data:/workspace"
);
```

### Generic Container Interface

#### micContainerExec()
Execute command in any container type.

```c
int micContainerExec(const char *containerType, const char *image, 
                     const char *command, const char *mounts);
```

**Parameters:**
- `containerType`: "singularity" or "docker"
- `image`: Container image path/name
- `command`: Command to execute
- `mounts`: Mount paths

**Example:**
```c
const char *type = use_singularity ? "singularity" : "docker";
micContainerExec(type, image, command, mounts);
```

### Container Best Practices

1. **Bind Mounts:** Always bind necessary data directories
2. **Absolute Paths:** Use absolute paths for images and mounts
3. **Error Handling:** Check return codes
4. **Resource Limits:** Use SLURM options for resource management
5. **Permissions:** Ensure proper file permissions in mounted directories

```c
// Good practice: Check if container image exists
if (micIsFileAvailable("my_container.sif")) {
    int result = micSingularityExec("my_container.sif", 
                                     "python script.py",
                                     "/data:/mnt");
    if (result == 0) {
        printf("Success!\n");
    }
} else {
    printf("Container image not found\n");
}
```

---

## Asynchronous Execution

Run commands in the background without blocking the main program.

### Functions

#### micAsyncExecute()
Start a command asynchronously.

```c
int micAsyncExecute(const char *command);
```

**Returns:** Process ID (PID) on success, -1 on failure

**Example:**
```c
// Start long-running task
int pid = micAsyncExecute("python long_computation.py > output.txt 2>&1");

if (pid > 0) {
    printf("Process started with PID %d\n", pid);
    
    // Continue with other work...
    doOtherWork();
}
```

#### micAsyncIsRunning()
Check if an async process is still running.

```c
bool micAsyncIsRunning(int pid);
```

**Example:**
```c
while (micAsyncIsRunning(pid)) {
    printf("Process %d still running...\n", pid);
    micWaitTime(1000);  // Check every second
}
```

#### micAsyncWait()
Wait for async process to complete.

```c
int micAsyncWait(int pid, int timeoutSec);
```

**Returns:** Exit code of process, or -1 on error/timeout

**Example:**
```c
// Wait up to 5 minutes
int exitCode = micAsyncWait(pid, 300);

if (exitCode == 0) {
    printf("Process completed successfully\n");
} else if (exitCode == -1) {
    printf("Process timed out or failed\n");
} else {
    printf("Process exited with code %d\n", exitCode);
}
```

#### micAsyncCancel()
Terminate an async process.

```c
bool micAsyncCancel(int pid);
```

**Example:**
```c
// Cancel if taking too long
if (elapsed > max_time) {
    micAsyncCancel(pid);
}
```

#### micAsyncGetOutput()
Read output from async process (from redirected file).

```c
char* micAsyncGetOutput(int pid, const char *outputFile);
```

**Example:**
```c
// Get output after completion
char *output = micAsyncGetOutput(pid, "process_output.txt");
if (output != NULL) {
    printf("Output:\n%s\n", output);
    micUnloadFileText(output);
}
```

### Async Execution Patterns

**Pattern 1: Fire and Forget**
```c
int pid = micAsyncExecute("cleanup.sh");
// Don't wait, let it run in background
```

**Pattern 2: Parallel Tasks**
```c
int pids[4];
pids[0] = micAsyncExecute("task1.sh");
pids[1] = micAsyncExecute("task2.sh");
pids[2] = micAsyncExecute("task3.sh");
pids[3] = micAsyncExecute("task4.sh");

// Wait for all
for (int i = 0; i < 4; i++) {
    micAsyncWait(pids[i], -1);  // Infinite wait
}
```

**Pattern 3: Timeout Handling**
```c
int pid = micAsyncExecute("long_task.sh > output.txt");

if (micAsyncWait(pid, 60) == -1) {
    printf("Timeout! Cancelling...\n");
    micAsyncCancel(pid);
} else {
    char *result = micAsyncGetOutput(pid, "output.txt");
    processResult(result);
    micUnloadFileText(result);
}
```

---

## Multithreading

Create and manage threads for parallel computation within a single process.

### Thread Creation

#### micThreadCreate()
Create and start a new thread.

```c
micThread* micThreadCreate(micThreadFunction func, void *arg);
```

**Parameters:**
- `func`: Function pointer to execute in thread
- `arg`: Argument to pass to function

**Returns:** Thread handle or NULL on failure

**Example:**
```c
void worker(void *arg) {
    int *data = (int *)arg;
    printf("Processing data: %d\n", *data);
    // Do work...
}

int data = 42;
micThread *thread = micThreadCreate(worker, &data);
```

#### micThreadJoin()
Wait for thread to complete.

```c
bool micThreadJoin(micThread *thread, int timeoutMs);
```

**Parameters:**
- `thread`: Thread handle
- `timeoutMs`: Timeout in milliseconds (-1 for infinite)

**Example:**
```c
// Wait indefinitely
micThreadJoin(thread, -1);

// Or with timeout
if (micThreadJoin(thread, 5000)) {
    printf("Thread completed\n");
} else {
    printf("Thread timeout\n");
}
```

#### micThreadDetach()
Detach thread (runs independently).

```c
void micThreadDetach(micThread *thread);
```

**Example:**
```c
micThread *t = micThreadCreate(background_task, NULL);
micThreadDetach(t);  // Let it run independently
```

### Thread Synchronization

#### Mutex Operations

```c
micMutex* micMutexCreate(void);
void micMutexDestroy(micMutex *mutex);
void micMutexLock(micMutex *mutex);
void micMutexUnlock(micMutex *mutex);
```

**Example:**
```c
typedef struct {
    int counter;
    micMutex *mutex;
} SharedData;

void increment_worker(void *arg) {
    SharedData *data = (SharedData *)arg;
    
    for (int i = 0; i < 1000; i++) {
        micMutexLock(data->mutex);
        data->counter++;
        micMutexUnlock(data->mutex);
    }
}

// Main code
SharedData shared = {0, micMutexCreate()};

micThread *t1 = micThreadCreate(increment_worker, &shared);
micThread *t2 = micThreadCreate(increment_worker, &shared);

micThreadJoin(t1, -1);
micThreadJoin(t2, -1);

printf("Final counter: %d\n", shared.counter);  // Should be 2000

micMutexDestroy(shared.mutex);
```

#### micGetNumCores()
Get number of CPU cores.

```c
int micGetNumCores(void);
```

**Example:**
```c
int cores = micGetNumCores();
printf("System has %d cores\n", cores);

// Create optimal number of threads
for (int i = 0; i < cores; i++) {
    threads[i] = micThreadCreate(worker, &data[i]);
}
```

### Multithreading Patterns

**Pattern 1: Parallel Data Processing**
```c
void process_chunk(void *arg) {
    ChunkData *chunk = (ChunkData *)arg;
    // Process data chunk
    for (int i = chunk->start; i < chunk->end; i++) {
        processItem(chunk->data[i]);
    }
}

// Split data into chunks
int numThreads = micGetNumCores();
ChunkData chunks[numThreads];
micThread *threads[numThreads];

// Create threads
for (int i = 0; i < numThreads; i++) {
    setupChunk(&chunks[i], data, i, numThreads);
    threads[i] = micThreadCreate(process_chunk, &chunks[i]);
}

// Wait for completion
for (int i = 0; i < numThreads; i++) {
    micThreadJoin(threads[i], -1);
}
```

**Pattern 2: Producer-Consumer**
```c
typedef struct {
    Queue *queue;
    micMutex *mutex;
    bool *done;
} WorkData;

void producer(void *arg) {
    WorkData *data = (WorkData *)arg;
    while (hasMoreWork()) {
        Item *item = createItem();
        
        micMutexLock(data->mutex);
        enqueue(data->queue, item);
        micMutexUnlock(data->mutex);
    }
    *data->done = true;
}

void consumer(void *arg) {
    WorkData *data = (WorkData *)arg;
    while (!*data->done || !isEmpty(data->queue)) {
        micMutexLock(data->mutex);
        if (!isEmpty(data->queue)) {
            Item *item = dequeue(data->queue);
            micMutexUnlock(data->mutex);
            processItem(item);
        } else {
            micMutexUnlock(data->mutex);
            micWaitTime(10);  // Brief pause
        }
    }
}

// Create producer and consumer threads
WorkData data = {queue, micMutexCreate(), &done};
micThread *prod = micThreadCreate(producer, &data);
micThread *cons = micThreadCreate(consumer, &data);
```

---

## Complete Examples

### Example 1: HPC Pipeline with SLURM and Containers

```c
void hpc_pipeline(void) {
    micBeginProcess("HPC Analysis Pipeline", MIC_LOG_INFO);
    
    // Step 1: Prepare data locally
    if (!micSkipStepIfComplete("prepare")) {
        micBeginStep("Prepare Data", MIC_LOG_INFO);
        prepareInputData();
        micMarkStepComplete("prepare");
        micEndStep();
    }
    
    // Step 2: Submit preprocessing to SLURM
    if (!micSkipStepIfComplete("preprocess")) {
        micBeginStep("Preprocess (SLURM)", MIC_LOG_INFO);
        
        int jobId = micSlurmSubmitJob(
            "preprocess.sh",
            "preprocess",
            "--nodes=1 --time=00:30:00"
        );
        
        if (micSlurmWaitForJob(jobId, 1800)) {
            micMarkStepComplete("preprocess");
        }
        micEndStep();
    }
    
    // Step 3: Run analysis in container
    if (!micSkipStepIfComplete("analyze")) {
        micBeginStep("Analyze (Container)", MIC_LOG_INFO);
        
        int result = micSingularityExec(
            "analysis_tool.sif",
            "analyze --input preprocessed.dat --output results.dat",
            "/data:/mnt"
        );
        
        if (result == 0) {
            micMarkStepComplete("analyze");
        }
        micEndStep();
    }
    
    // Step 4: Collect results
    if (!micSkipStepIfComplete("collect")) {
        micBeginStep("Collect Results", MIC_LOG_INFO);
        collectAndSaveResults();
        micMarkStepComplete("collect");
        micEndStep();
    }
    
    micEndProcess();
}
```

### Example 2: Parallel Processing with Threads

```c
typedef struct {
    char **files;
    int start;
    int end;
    int *processedCount;
    micMutex *mutex;
} ProcessingData;

void process_files_thread(void *arg) {
    ProcessingData *data = (ProcessingData *)arg;
    
    for (int i = data->start; i < data->end; i++) {
        processFile(data->files[i]);
        
        micMutexLock(data->mutex);
        (*data->processedCount)++;
        micMutexUnlock(data->mutex);
    }
}

void parallel_file_processing(char **files, int numFiles) {
    int numThreads = micGetNumCores();
    int filesPerThread = numFiles / numThreads;
    
    int processedCount = 0;
    micMutex *mutex = micMutexCreate();
    
    ProcessingData data[numThreads];
    micThread *threads[numThreads];
    
    // Create threads
    for (int i = 0; i < numThreads; i++) {
        data[i].files = files;
        data[i].start = i * filesPerThread;
        data[i].end = (i == numThreads - 1) ? numFiles : (i + 1) * filesPerThread;
        data[i].processedCount = &processedCount;
        data[i].mutex = mutex;
        
        threads[i] = micThreadCreate(process_files_thread, &data[i]);
    }
    
    // Wait for completion
    for (int i = 0; i < numThreads; i++) {
        micThreadJoin(threads[i], -1);
    }
    
    printf("Processed %d files\n", processedCount);
    micMutexDestroy(mutex);
}
```

---

## Compilation

### Basic Compilation (with threading)
```bash
gcc -o myapp myapp.c -lz -lpthread
```

### Without Threading
```bash
gcc -o myapp myapp.c -lz -DMIC_NO_THREADS
```

### Without Compression
```bash
gcc -o myapp myapp.c -lpthread -DMIC_NO_ZLIB
```

### Minimal Build (no HPC features)
```bash
gcc -o myapp myapp.c -DMIC_NO_THREADS -DMIC_NO_ZLIB
```

---

## Best Practices

### SLURM
1. **Always check job submission success** before waiting
2. **Use appropriate timeouts** based on expected runtime
3. **Monitor job status** periodically, not continuously
4. **Clean up output files** after processing
5. **Handle job failures** gracefully

### Containers
1. **Use absolute paths** for images and bind mounts
2. **Verify container images exist** before execution
3. **Set resource limits** through SLURM when possible
4. **Test locally** before deploying to cluster
5. **Handle missing containers** gracefully

### Async Execution
1. **Redirect output to files** for later retrieval
2. **Set reasonable timeouts** to avoid hanging
3. **Clean up processes** on timeout or error
4. **Track PIDs** for process management
5. **Handle process failures** appropriately

### Multithreading
1. **Always use mutexes** for shared data
2. **Avoid data races** through proper synchronization
3. **Join threads** before accessing results
4. **Scale thread count** based on CPU cores
5. **Handle thread creation failures**
6. **Minimize lock contention** for performance

### General
1. **Check return values** from all functions
2. **Free allocated memory** (output strings, threads, mutexes)
3. **Use logging** for debugging HPC workflows
4. **Test on local system** before deploying to cluster
5. **Document resource requirements** in scripts

---

## Troubleshooting

### SLURM Commands Not Found
```
ERROR: sbatch: not found
```
**Solution:** Ensure SLURM is installed and in PATH, or run on HPC cluster

### Container Runtime Missing
```
ERROR: singularity: not found
```
**Solution:** Install Singularity/Docker or use graceful fallback

### Thread Creation Failed
```
ERROR: Failed to create thread
```
**Solution:** Check system thread limits (`ulimit -u`) or reduce thread count

### Async Process Timeout
```
WARNING: Timeout waiting for async process
```
**Solution:** Increase timeout or check process is actually running

---

## See Also
- [README.md](README.md) - Library overview
- [ADVANCED_FEATURES.md](ADVANCED_FEATURES.md) - Compression and pipeline features
- [hpc_example.c](hpc_example.c) - Complete HPC examples
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Implementation details
