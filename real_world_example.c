/*
 * Real-World Pipeline Example: Data Processing ETL
 * 
 * This example demonstrates a realistic ETL (Extract, Transform, Load) pipeline
 * that processes CSV data files with compression and error handling.
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

// Simulated functions (in real use, these would do actual work)
void downloadDataFromAPI(const char *url, const char *output) {
    // Simulate downloading data
    const char *sampleData = "id,name,value\n1,Alice,100\n2,Bob,200\n3,Charlie,300\n";
    micSaveFileText(output, (char *)sampleData);
}

void transformCSVData(const char *input, const char *output) {
    // Load, transform, and save
    char *data = micLoadFileText(input);
    if (data != NULL) {
        const char *transformed = micStringToUpper(data);
        micSaveFileText(output, (char *)transformed);
        micUnloadFileText(data);
    }
}

int main(void) {
    printf("═══════════════════════════════════════════════════════\n");
    printf("║  Real-World ETL Pipeline Example                    ║\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // =====================================================================
    // FIRST RUN: Execute complete pipeline
    // =====================================================================
    printf(">>> FIRST RUN: Complete Pipeline Execution\n\n");
    
    micBeginProcess("ETL Pipeline v1.0", MIC_LOG_INFO);
    
    // ---------------------------------------------------------------
    // STEP 1: Extract - Download data from API
    // ---------------------------------------------------------------
    const char *extractStep = "extract_data";
    if (!micSkipStepIfComplete(extractStep)) {
        micBeginStep("Extract: Download from API", MIC_LOG_INFO);
        
        // Check if API credentials are available
        const char *credentials[] = {"api_key.txt"};
        
        // For demo, create a fake credentials file
        micSaveFileText("api_key.txt", "demo_key_12345");
        
        if (micCheckDependencies(credentials, 1)) {
            printf("  → Downloading data from API...\n");
            downloadDataFromAPI("https://api.example.com/data", "raw_data.csv");
            
            // Wait for download to complete (with 10 second timeout)
            if (micWaitForFile("raw_data.csv", 10000)) {
                printf("  ✓ Download complete: raw_data.csv\n");
                micMarkStepComplete(extractStep);
            } else {
                printf("  ✗ Download timeout!\n");
                return 1;
            }
        } else {
            printf("  ✗ Missing API credentials!\n");
            return 1;
        }
        
        micEndStep();
    }
    printf("\n");
    
    // ---------------------------------------------------------------
    // STEP 2: Transform - Process and clean data
    // ---------------------------------------------------------------
    const char *transformStep = "transform_data";
    if (!micSkipStepIfComplete(transformStep)) {
        micBeginStep("Transform: Process Data", MIC_LOG_INFO);
        
        // Check dependencies
        const char *deps[] = {"raw_data.csv"};
        if (micCheckDependencies(deps, 1)) {
            printf("  → Transforming data...\n");
            transformCSVData("raw_data.csv", "transformed_data.csv");
            printf("  ✓ Transformation complete: transformed_data.csv\n");
            
            micMarkStepComplete(transformStep);
        } else {
            printf("  ✗ Missing input data!\n");
            return 1;
        }
        
        micEndStep();
    }
    printf("\n");
    
    // ---------------------------------------------------------------
    // STEP 3: Load - Compress and archive data
    // ---------------------------------------------------------------
    const char *loadStep = "load_archive";
    if (!micSkipStepIfComplete(loadStep)) {
        micBeginStep("Load: Compress and Archive", MIC_LOG_INFO);
        
        // Load processed data
        unsigned int dataSize = 0;
        unsigned char *data = micLoadFileData("transformed_data.csv", &dataSize);
        
        if (data != NULL) {
            printf("  → Compressing %u bytes...\n", dataSize);
            
            // Compress the data
            int compressedSize = 0;
            unsigned char *compressed = micCompressData(data, dataSize, &compressedSize);
            
            if (compressed != NULL) {
                // Save compressed archive
                micSaveFileData("archive.dat.z", compressed, compressedSize);
                printf("  ✓ Archive created: archive.dat.z\n");
                printf("    Compression: %u → %d bytes (%.1f%% saved)\n",
                       dataSize, compressedSize,
                       100.0 * (1.0 - (double)compressedSize / dataSize));
                
                micUnloadFileData(compressed);
                micMarkStepComplete(loadStep);
            } else {
                printf("  ✗ Compression failed!\n");
            }
            
            micUnloadFileData(data);
        } else {
            printf("  ✗ Failed to load data!\n");
        }
        
        micEndStep();
    }
    printf("\n");
    
    micEndProcess();
    
    // =====================================================================
    // SECOND RUN: All steps should be skipped
    // =====================================================================
    printf("\n>>> SECOND RUN: All Steps Should Skip\n\n");
    
    micBeginProcess("ETL Pipeline v1.0 (Rerun)", MIC_LOG_INFO);
    
    int stepsExecuted = 0;
    
    if (!micSkipStepIfComplete(extractStep)) {
        printf("  → Extract step executed\n");
        stepsExecuted++;
    }
    
    if (!micSkipStepIfComplete(transformStep)) {
        printf("  → Transform step executed\n");
        stepsExecuted++;
    }
    
    if (!micSkipStepIfComplete(loadStep)) {
        printf("  → Load step executed\n");
        stepsExecuted++;
    }
    
    if (stepsExecuted == 0) {
        printf("  ✓ All steps skipped - pipeline is up to date!\n");
    } else {
        printf("  ⚠ Warning: %d steps re-executed\n", stepsExecuted);
    }
    
    micEndProcess();
    
    // =====================================================================
    // VERIFY OUTPUT: Decompress and check archive
    // =====================================================================
    printf("\n>>> Verifying Archive Integrity\n\n");
    
    if (micIsFileAvailable("archive.dat.z")) {
        unsigned int compSize = 0;
        unsigned char *compData = micLoadFileData("archive.dat.z", &compSize);
        
        if (compData != NULL) {
            printf("  → Decompressing archive...\n");
            
            int origSize = 0;
            unsigned char *decompData = micDecompressData(compData, compSize, &origSize);
            
            if (decompData != NULL) {
                printf("  ✓ Decompression successful: %d bytes\n", origSize);
                printf("  → First 50 chars: %.50s...\n", decompData);
                
                micUnloadFileData(decompData);
            }
            
            micUnloadFileData(compData);
        }
    }
    
    // =====================================================================
    // CLEANUP
    // =====================================================================
    printf("\n>>> Cleanup\n\n");
    
    micDeleteFile("api_key.txt");
    micDeleteFile("raw_data.csv");
    micDeleteFile("transformed_data.csv");
    micDeleteFile("archive.dat.z");
    micClearAllStepStates();
    
    printf("  ✓ All temporary files and states cleaned up\n");
    
    printf("\n═══════════════════════════════════════════════════════\n");
    printf("║  Pipeline Complete!                                  ║\n");
    printf("═══════════════════════════════════════════════════════\n");
    
    return 0;
}
