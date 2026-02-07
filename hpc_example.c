/*
 * MIC Library - HPC Features Example
 * Demonstrates SLURM, containers, async execution, and multithreading
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

// ============================================================================
// Example 1: SLURM Job Submission
// ============================================================================
void example_slurm(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 1: SLURM Job Submission                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Create a simple SLURM script
    const char *scriptContent = 
        "#!/bin/bash\n"
        "#SBATCH --ntasks=1\n"
        "#SBATCH --time=00:05:00\n"
        "#SBATCH --mem=1G\n"
        "\n"
        "echo 'Hello from SLURM job!'\n"
        "sleep 2\n"
        "echo 'Job completed'\n";
    
    const char *scriptFile = "test_job.sh";
    if (micSaveFileText(scriptFile, (char *)scriptContent))
    {
        printf("✓ SLURM script created: %s\n\n", scriptFile);
        
        // Submit job
        int jobId = micSlurmSubmitJob(scriptFile, "mic_test_job", "--output=job_output.txt");
        
        if (jobId > 0)
        {
            printf("✓ Job submitted with ID: %d\n", jobId);
            
            // Check status
            int status = micSlurmJobStatus(jobId);
            if (status == 0)
            {
                printf("  Status: RUNNING or PENDING\n");
            }
            
            // Note: Actual waiting would be done in production
            printf("\n  To wait for job completion:\n");
            printf("    bool success = micSlurmWaitForJob(%d, 300);\n", jobId);
            printf("\n  To cancel job:\n");
            printf("    micSlurmCancelJob(%d);\n", jobId);
            printf("\n  To get output:\n");
            printf("    char *output = micSlurmGetJobOutput(%d, \"job_output.txt\");\n", jobId);
        }
        else
        {
            printf("✗ Job submission failed (SLURM may not be available)\n");
            printf("  This is expected if not running on an HPC system\n");
        }
        
        // Cleanup
        micDeleteFile(scriptFile);
    }
}

// ============================================================================
// Example 2: Container Execution
// ============================================================================
void example_containers(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 2: Container Execution                           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Singularity Example:\n");
    printf("  Command: micSingularityExec(\"ubuntu.sif\", \"ls -la\", \"/data:/mnt\")\n");
    printf("  This would execute 'ls -la' inside ubuntu.sif container\n");
    printf("  with /data mounted to /mnt\n\n");
    
    printf("Docker Example:\n");
    printf("  Command: micDockerRun(\"ubuntu:latest\", \"echo 'Hello'\", \"/data:/data\")\n");
    printf("  This would run 'echo Hello' in ubuntu:latest container\n");
    printf("  with /data volume mounted\n\n");
    
    printf("Generic Container:\n");
    printf("  Command: micContainerExec(\"singularity\", \"image.sif\", \"pwd\", NULL)\n");
    printf("  Automatically selects container runtime\n\n");
    
    // Try a simple container command (will fail if not available)
    printf("Attempting Docker hello-world (may fail if Docker not available):\n");
    int result = micDockerRun("hello-world", NULL, NULL);
    if (result == 0)
    {
        printf("✓ Docker execution succeeded\n");
    }
    else
    {
        printf("✗ Docker execution failed (Docker may not be available)\n");
        printf("  This is expected if Docker is not installed\n");
    }
}

// ============================================================================
// Example 3: Asynchronous Execution
// ============================================================================
void example_async(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 3: Asynchronous Execution                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Start an async command that writes to a file
    const char *command = "echo 'Async task running...' > async_output.txt && sleep 1 && echo 'Done!' >> async_output.txt";
    
    printf("Starting async command: %s\n", command);
    int pid = micAsyncExecute(command);
    
    if (pid > 0)
    {
        printf("✓ Async process started with PID: %d\n\n", pid);
        
        // Check if running
        if (micAsyncIsRunning(pid))
        {
            printf("  Process is running...\n");
        }
        
        // Wait for completion
        printf("  Waiting for completion (timeout: 5 sec)...\n");
        int exitCode = micAsyncWait(pid, 5);
        
        if (exitCode == 0)
        {
            printf("✓ Process completed successfully\n\n");
            
            // Read output
            char *output = micAsyncGetOutput(pid, "async_output.txt");
            if (output != NULL)
            {
                printf("  Output:\n%s\n", output);
                micUnloadFileText(output);
            }
            
            micDeleteFile("async_output.txt");
        }
        else if (exitCode < 0)
        {
            printf("✗ Process failed or timed out\n");
        }
    }
    else
    {
        printf("✗ Failed to start async process\n");
    }
}

// ============================================================================
// Example 4: Multithreading
// ============================================================================

#if !defined(MIC_NO_THREADS)

// Shared data structure for thread example
typedef struct {
    int id;
    int iterations;
    int *counter;
    micMutex *mutex;
} ThreadData;

// Thread function that increments a shared counter
void worker_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    
    printf("  Thread %d starting (%d iterations)\n", data->id, data->iterations);
    
    for (int i = 0; i < data->iterations; i++)
    {
        // Lock mutex before modifying shared data
        micMutexLock(data->mutex);
        (*data->counter)++;
        micMutexUnlock(data->mutex);
        
        // Simulate some work
        micWaitTime(10);
    }
    
    printf("  Thread %d finished\n", data->id);
}

void example_multithreading(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 4: Multithreading                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    int numCores = micGetNumCores();
    printf("System has %d CPU cores\n\n", numCores);
    
    // Create shared counter and mutex
    int counter = 0;
    micMutex *mutex = micMutexCreate();
    
    if (mutex == NULL)
    {
        printf("✗ Failed to create mutex\n");
        return;
    }
    
    // Create thread data
    const int NUM_THREADS = 4;
    const int ITERATIONS = 10;
    ThreadData threadData[NUM_THREADS];
    micThread *threads[NUM_THREADS];
    
    printf("Starting %d threads, each doing %d iterations\n", NUM_THREADS, ITERATIONS);
    printf("Expected final counter value: %d\n\n", NUM_THREADS * ITERATIONS);
    
    // Create and start threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadData[i].id = i + 1;
        threadData[i].iterations = ITERATIONS;
        threadData[i].counter = &counter;
        threadData[i].mutex = mutex;
        
        threads[i] = micThreadCreate(worker_thread, &threadData[i]);
        
        if (threads[i] == NULL)
        {
            printf("✗ Failed to create thread %d\n", i + 1);
        }
    }
    
    // Wait for all threads to complete
    printf("\nWaiting for threads to complete...\n\n");
    
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (threads[i] != NULL)
        {
            micThreadJoin(threads[i], -1);  // Infinite wait
        }
    }
    
    printf("\n✓ All threads completed\n");
    printf("  Final counter value: %d\n", counter);
    
    if (counter == NUM_THREADS * ITERATIONS)
    {
        printf("  ✓ Counter is correct! Thread synchronization worked.\n");
    }
    else
    {
        printf("  ✗ Counter mismatch! Expected %d, got %d\n", NUM_THREADS * ITERATIONS, counter);
    }
    
    // Cleanup
    micMutexDestroy(mutex);
}

#else

void example_multithreading(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 4: Multithreading                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Multithreading is disabled (MIC_NO_THREADS defined)\n");
    printf("Compile without -DMIC_NO_THREADS and with -lpthread to enable\n");
}

#endif

// ============================================================================
// Example 5: Complete HPC Pipeline
// ============================================================================
void example_hpc_pipeline(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example 5: Complete HPC Pipeline                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    micBeginProcess("HPC Pipeline Demo", MIC_LOG_INFO);
    
    // Step 1: Prepare data (with step skipping)
    const char *prepareStep = "prepare_data";
    if (!micSkipStepIfComplete(prepareStep))
    {
        micBeginStep("Step 1: Prepare Data", MIC_LOG_INFO);
        
        printf("  → Creating input data...\n");
        const char *inputData = "Sample data for HPC processing\n";
        micSaveFileText("hpc_input.txt", (char *)inputData);
        
        micMarkStepComplete(prepareStep);
        micEndStep();
    }
    
    // Step 2: Process with container (async)
    const char *processStep = "process_data";
    if (!micSkipStepIfComplete(processStep))
    {
        micBeginStep("Step 2: Process with Container", MIC_LOG_INFO);
        
        printf("  → Processing data in container (simulated)...\n");
        
        // In real scenario: micSingularityExec("my_tool.sif", "process hpc_input.txt", "/data:/data")
        // For demo, just transform the data
        char *input = micLoadFileText("hpc_input.txt");
        if (input != NULL)
        {
            const char *processed = micStringToUpper(input);
            micSaveFileText("hpc_processed.txt", (char *)processed);
            micUnloadFileText(input);
        }
        
        micMarkStepComplete(processStep);
        micEndStep();
    }
    
    // Step 3: Submit to SLURM for heavy computation
    const char *computeStep = "heavy_compute";
    if (!micSkipStepIfComplete(computeStep))
    {
        micBeginStep("Step 3: Heavy Computation", MIC_LOG_INFO);
        
        printf("  → Heavy computation step (simulated)...\n");
        printf("    In production: submit to SLURM cluster\n");
        printf("    micSlurmSubmitJob(\"compute_script.sh\", \"computation\", \"--nodes=4\")\n");
        
        // Simulate computation result
        micSaveFileText("hpc_result.txt", "COMPUTATION RESULT: 42\n");
        
        micMarkStepComplete(computeStep);
        micEndStep();
    }
    
    // Step 4: Collect results
    const char *collectStep = "collect_results";
    if (!micSkipStepIfComplete(collectStep))
    {
        micBeginStep("Step 4: Collect Results", MIC_LOG_INFO);
        
        const char *deps[] = {"hpc_processed.txt", "hpc_result.txt"};
        if (micCheckDependencies(deps, 2))
        {
            printf("  → Collecting results...\n");
            
            char *processed = micLoadFileText("hpc_processed.txt");
            char *result = micLoadFileText("hpc_result.txt");
            
            if (processed != NULL && result != NULL)
            {
                printf("\n  Final Results:\n");
                printf("  ───────────────────────────────────────────────────\n");
                printf("  Processed: %s", processed);
                printf("  Result: %s", result);
                printf("  ───────────────────────────────────────────────────\n");
                
                micUnloadFileText(processed);
                micUnloadFileText(result);
            }
            
            micMarkStepComplete(collectStep);
        }
        
        micEndStep();
    }
    
    micEndProcess();
    
    // Cleanup
    printf("\n  Cleaning up temporary files...\n");
    micDeleteFile("hpc_input.txt");
    micDeleteFile("hpc_processed.txt");
    micDeleteFile("hpc_result.txt");
    
    printf("✓ HPC Pipeline completed!\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void)
{
    printf("╔════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                 MIC Library - HPC Features Demo                       ║\n");
    printf("║                                                                        ║\n");
    printf("║  Demonstrates:                                                         ║\n");
    printf("║  - SLURM job submission and management                                ║\n");
    printf("║  - Container execution (Singularity & Docker)                         ║\n");
    printf("║  - Asynchronous task execution                                        ║\n");
    printf("║  - Multithreading with synchronization                                ║\n");
    printf("║  - Complete HPC pipeline workflow                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════╝\n");
    
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // Run examples
    example_slurm();
    example_containers();
    example_async();
    example_multithreading();
    example_hpc_pipeline();
    
    printf("\n╔════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  All HPC Examples Complete!                                           ║\n");
    printf("║                                                                        ║\n");
    printf("║  Note: Some examples may show failures if HPC tools (SLURM, Docker,   ║\n");
    printf("║  Singularity) are not installed. This is expected on non-HPC systems. ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
