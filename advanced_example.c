/*
 * MIC Library - Advanced Pipeline and Compression Example
 * Demonstrates compression functions and advanced pipeline features
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

void testCompression(void)
{
    printf("\n=== Testing Compression ===\n");
    
    // Create test data
    const char *testData = "Hello, World! This is a test of the MIC compression functions. "
                          "We're using zlib to compress and decompress data. "
                          "The quick brown fox jumps over the lazy dog. "
                          "This text should compress quite well due to repetition. "
                          "Hello, World! Hello, World! Hello, World!";
    
    int originalSize = strlen(testData);
    printf("Original data: %d bytes\n", originalSize);
    printf("Original text: %.50s...\n", testData);
    
    // Compress data
    int compressedSize = 0;
    unsigned char *compressed = micCompressData((unsigned char *)testData, originalSize, &compressedSize);
    
    if (compressed != NULL)
    {
        printf("\nCompression successful!\n");
        printf("Compressed size: %d bytes (%.1f%% of original)\n", 
               compressedSize, 100.0 * compressedSize / originalSize);
        
        // Decompress data
        int decompressedSize = 0;
        unsigned char *decompressed = micDecompressData(compressed, compressedSize, &decompressedSize);
        
        if (decompressed != NULL)
        {
            printf("\nDecompression successful!\n");
            printf("Decompressed size: %d bytes\n", decompressedSize);
            
            // Verify data integrity
            if (decompressedSize == originalSize && 
                memcmp(testData, decompressed, originalSize) == 0)
            {
                printf("✓ Data integrity verified - decompressed data matches original!\n");
                printf("Decompressed text: %.50s...\n", decompressed);
            }
            else
            {
                printf("✗ Data integrity check failed!\n");
            }
            
            micUnloadFileData(decompressed);
        }
        
        micUnloadFileData(compressed);
    }
}

void testAdvancedPipeline(void)
{
    printf("\n\n=== Testing Advanced Pipeline Features ===\n");
    
    // Initialize logging
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // Define a simple data pipeline
    printf("\n--- Running Pipeline (First Time) ---\n");
    
    micBeginProcess("Data Processing Pipeline", MIC_LOG_INFO);
    
    // Step 1: Download/Create Input Data
    const char *step1 = "download_data";
    if (!micSkipStepIfComplete(step1))
    {
        micBeginStep("Step 1: Download Data", MIC_LOG_INFO);
        
        // Simulate creating input file
        const char *inputFile = "pipeline_input.txt";
        micSaveFileText(inputFile, "Input data for pipeline processing\n");
        printf("Created input file: %s\n", inputFile);
        
        micMarkStepComplete(step1);
        micEndStep();
    }
    
    // Step 2: Process Data (with dependency check)
    const char *step2 = "process_data";
    if (!micSkipStepIfComplete(step2))
    {
        micBeginStep("Step 2: Process Data", MIC_LOG_INFO);
        
        // Check dependencies
        const char *deps[] = {"pipeline_input.txt"};
        if (micCheckDependencies(deps, 1))
        {
            // Read input
            char *input = micLoadFileText("pipeline_input.txt");
            if (input != NULL)
            {
                // Process (uppercase)
                const char *processed = micStringToUpper(input);
                micSaveFileText("pipeline_processed.txt", (char *)processed);
                printf("Processed data saved\n");
                micUnloadFileText(input);
            }
            
            micMarkStepComplete(step2);
        }
        else
        {
            printf("Dependencies not satisfied, skipping step\n");
        }
        
        micEndStep();
    }
    
    // Step 3: Generate Output (with file polling)
    const char *step3 = "generate_output";
    if (!micSkipStepIfComplete(step3))
    {
        micBeginStep("Step 3: Generate Output", MIC_LOG_INFO);
        
        // Wait for processed file (should exist immediately)
        if (micWaitForFile("pipeline_processed.txt", 5000))
        {
            char *processed = micLoadFileText("pipeline_processed.txt");
            if (processed != NULL)
            {
                // Add timestamp and save
                long timestamp = micGetTimeStamp();
                const char *output = micStringFormat("%s\nProcessed at: %ld\n", 
                                                     processed, timestamp);
                micSaveFileText("pipeline_output.txt", (char *)output);
                printf("Final output generated\n");
                micUnloadFileText(processed);
            }
            
            micMarkStepComplete(step3);
        }
        
        micEndStep();
    }
    
    micEndProcess();
    
    // Run pipeline again - should skip all completed steps
    printf("\n--- Running Pipeline (Second Time - Should Skip) ---\n");
    
    micBeginProcess("Data Processing Pipeline (Rerun)", MIC_LOG_INFO);
    
    if (!micSkipStepIfComplete(step1))
    {
        printf("Step 1 executed again\n");
    }
    
    if (!micSkipStepIfComplete(step2))
    {
        printf("Step 2 executed again\n");
    }
    
    if (!micSkipStepIfComplete(step3))
    {
        printf("Step 3 executed again\n");
    }
    
    micEndProcess();
    
    // Test clearing specific step
    printf("\n--- Clearing Step 2 State ---\n");
    micClearStepState(step2);
    
    // Run pipeline again - only step 2 should execute
    printf("\n--- Running Pipeline (Third Time - Only Step 2) ---\n");
    
    micBeginProcess("Data Processing Pipeline (Partial Rerun)", MIC_LOG_INFO);
    
    if (!micSkipStepIfComplete(step1))
    {
        printf("Step 1 executed\n");
    }
    
    if (!micSkipStepIfComplete(step2))
    {
        micBeginStep("Step 2: Process Data (Rerun)", MIC_LOG_INFO);
        printf("Step 2 re-executed after clearing state\n");
        micMarkStepComplete(step2);
        micEndStep();
    }
    
    if (!micSkipStepIfComplete(step3))
    {
        printf("Step 3 executed\n");
    }
    
    micEndProcess();
    
    // Cleanup
    printf("\n--- Cleanup ---\n");
    micDeleteFile("pipeline_input.txt");
    micDeleteFile("pipeline_processed.txt");
    micDeleteFile("pipeline_output.txt");
    micClearAllStepStates();
    printf("Pipeline files and states cleaned up\n");
}

void testFilePolling(void)
{
    printf("\n\n=== Testing File Polling ===\n");
    
    // Test waiting for a file that will be created
    const char *testFile = "delayed_file.txt";
    
    printf("Testing micWaitForFile with 2 second timeout...\n");
    
    // Create file in background (simulated by immediate creation)
    printf("Creating file after short delay...\n");
    micWaitTime(500);  // Wait 500ms
    micSaveFileText(testFile, "This file was created with a delay\n");
    
    // Now wait for it
    if (micWaitForFile(testFile, 3000))
    {
        printf("✓ File found successfully!\n");
    }
    else
    {
        printf("✗ Timeout waiting for file\n");
    }
    
    // Test waiting for multiple files
    printf("\nTesting micWaitForFiles with multiple files...\n");
    const char *files[] = {testFile, "pipeline_input.txt", "pipeline_output.txt"};
    
    // Create missing files
    micSaveFileText("pipeline_input.txt", "test\n");
    micSaveFileText("pipeline_output.txt", "test\n");
    
    if (micWaitForFiles(files, 3, 2000))
    {
        printf("✓ All files found!\n");
    }
    else
    {
        printf("✗ Timeout waiting for files\n");
    }
    
    // Cleanup
    micDeleteFile(testFile);
    micDeleteFile("pipeline_input.txt");
    micDeleteFile("pipeline_output.txt");
}

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  MIC Library - Advanced Pipeline & Compression Demo         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // Test compression functions
    testCompression();
    
    // Test advanced pipeline features
    testAdvancedPipeline();
    
    // Test file polling
    testFilePolling();
    
    printf("\n\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  All Tests Complete!                                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
