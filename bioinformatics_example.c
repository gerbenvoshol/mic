/*
 * MIC Library - Bioinformatics Workflow Example
 * Demonstrates the new bioinformatics workflow functionality
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

int main(void)
{
    // Initialize
    micSetTraceLogLevel(MIC_LOG_INFO);
    micTraceLog(MIC_LOG_INFO, "=== MIC Bioinformatics Workflow Example ===\n");
    
    // ====================================
    // Configuration Management
    // ====================================
    micTraceLog(MIC_LOG_INFO, "--- Configuration Management ---");
    
    // Create a test config file
    const char *configContent = 
        "# Test configuration\n"
        "workdir=/tmp/pipeline\n"
        "threads=4\n"
        "quality_threshold=30\n"
        "output_format=fastq\n";
    
    micSaveFileText("test_config.txt", (char *)configContent);
    
    micConfig config;
    config.count = 0;
    
    if (micLoadConfig(&config, "test_config.txt") > 0)
    {
        printf("Loaded configuration with %d entries\n", config.count);
        printf("workdir: %s\n", micConfigGet(&config, "workdir"));
        printf("threads: %s\n", micConfigGet(&config, "threads"));
        printf("quality_threshold: %s\n", micConfigGet(&config, "quality_threshold"));
        
        // Test default value
        printf("unknown_key (with default): %s\n", 
               micConfigGetDefault(&config, "unknown_key", "default_value"));
        
        // Test set
        micConfigSet(&config, "new_param", "test_value");
        printf("new_param: %s\n", micConfigGet(&config, "new_param"));
    }
    
    // Test JSON config
    const char *jsonConfig = 
        "{\n"
        "  \"database\": \"genome.db\",\n"
        "  \"reference\": \"hg38\",\n"
        "  \"max_memory\": \"16G\"\n"
        "}\n";
    
    micSaveFileText("test_config.json", (char *)jsonConfig);
    
    micConfig jsonCfg;
    jsonCfg.count = 0;
    
    if (micLoadConfigJSON(&jsonCfg, "test_config.json") > 0)
    {
        printf("\nJSON config loaded with %d entries\n", jsonCfg.count);
        printf("database: %s\n", micConfigGet(&jsonCfg, "database"));
        printf("reference: %s\n", micConfigGet(&jsonCfg, "reference"));
    }
    
    // ====================================
    // Sample List Management
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Sample List Management ---");
    
    // Create sample list file
    const char *samplesContent = 
        "sample1\n"
        "sample2\n"
        "sample3\n"
        "control1\n";
    
    micSaveFileText("samples.txt", (char *)samplesContent);
    
    micSampleList samples;
    samples.samples = NULL;
    samples.count = 0;
    
    if (micLoadSamplesList(&samples, "samples.txt") > 0)
    {
        printf("Loaded %d samples\n", samples.count);
        for (int i = 0; i < samples.count; i++)
        {
            printf("  Sample %d: %s\n", i, micGetSample(&samples, i));
        }
    }
    
    // Test JSON samples
    const char *jsonSamples = "[\"sampleA\", \"sampleB\", \"sampleC\"]";
    micSaveFileText("samples.json", (char *)jsonSamples);
    
    micSampleList jsonSampleList;
    jsonSampleList.samples = NULL;
    jsonSampleList.count = 0;
    
    if (micLoadSamplesJSON(&jsonSampleList, "samples.json") > 0)
    {
        printf("\nJSON samples loaded: %d\n", jsonSampleList.count);
        for (int i = 0; i < jsonSampleList.count; i++)
        {
            printf("  Sample: %s\n", micGetSample(&jsonSampleList, i));
        }
    }
    
    // ====================================
    // File Pattern Expansion
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- File Pattern Expansion ---");
    
    int expandedCount = 0;
    char **expandedFiles = micExpand("output/{sample}_R1.fastq", &samples, &expandedCount);
    
    if (expandedFiles != NULL)
    {
        printf("Expanded pattern to %d files:\n", expandedCount);
        for (int i = 0; i < expandedCount; i++)
        {
            printf("  %s\n", expandedFiles[i]);
        }
        micFreeExpandedFiles(expandedFiles, expandedCount);
    }
    
    // Test wildcard functions
    const char *pattern = "metrics/{sample}_R1.stats";
    const char *filename = "metrics/sample1_R1.stats";
    
    char *extractedSample = micExtractWildcard(pattern, filename, "sample");
    if (extractedSample != NULL)
    {
        printf("\nExtracted wildcard 'sample' from '%s': %s\n", filename, extractedSample);
        MIC_FREE(extractedSample);
    }
    
    char *replaced = micReplaceWildcard(pattern, "sample", "test123");
    if (replaced != NULL)
    {
        printf("Replaced wildcard: %s\n", replaced);
        MIC_FREE(replaced);
    }
    
    // ====================================
    // CSV Operations
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- CSV File Operations ---");
    
    // Create test CSV
    const char *csvContent = 
        "Sample,ReadCount,QualityScore,GCContent\n"
        "sample1,1000000,35.5,0.48\n"
        "sample2,1200000,34.2,0.52\n"
        "sample3,980000,36.1,0.49\n"
        "control1,1100000,35.8,0.50\n";
    
    micSaveFileText("test_data.csv", (char *)csvContent);
    
    micCSVFile *csv = micLoadCSV("test_data.csv", ',', true);
    
    if (csv != NULL)
    {
        printf("CSV loaded: %d rows, %d columns\n", csv->rowCount, csv->headerCount);
        
        // Print headers
        printf("Headers: ");
        for (int i = 0; i < csv->headerCount; i++)
        {
            printf("%s ", csv->headers[i]);
        }
        printf("\n");
        
        // Print first row
        printf("First row: ");
        for (int i = 0; i < csv->rows[0].fieldCount; i++)
        {
            printf("%s ", csv->rows[0].fields[i]);
        }
        printf("\n");
        
        // Test get value by name
        const char *value = micCSVGetValueByName(csv, 0, "ReadCount");
        printf("ReadCount for first sample: %s\n", value);
        
        // Test sum column
        double totalReads = micCSVSumColumn(csv, 1);  // Column index 1 is ReadCount
        printf("Total reads across all samples: %.0f\n", totalReads);
        
        // Test count where
        int controlCount = micCSVCountWhere(csv, 0, "control1");
        printf("Number of control1 samples: %d\n", controlCount);
        
        // Save modified CSV
        micSaveCSV("test_output.csv", csv);
        printf("Saved CSV to test_output.csv\n");
        
        micFreeCSV(csv);
    }
    
    // Test creating CSV from scratch
    const char *headers[] = {"Gene", "Expression", "Pvalue"};
    micCSVFile *newCsv = micCSVCreate(headers, 3, ',');
    
    if (newCsv != NULL)
    {
        const char *row1[] = {"GeneA", "5.2", "0.001"};
        const char *row2[] = {"GeneB", "3.8", "0.05"};
        
        micCSVAddRow(newCsv, row1, 3);
        micCSVAddRow(newCsv, row2, 3);
        
        printf("\nCreated new CSV with %d rows\n", newCsv->rowCount);
        micSaveCSV("gene_expression.csv", newCsv);
        
        micFreeCSV(newCsv);
    }
    
    // ====================================
    // Text Processing (AWK-like)
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Text Processing ---");
    
    const char *textData = 
        "sample1\t100\t200\t1.5\n"
        "sample2\t150\t180\t2.1\n"
        "sample3\t120\t220\t1.8\n"
        "sample4\t90\t190\t1.2\n";
    
    // Count lines
    int lineCount = micCountLines(textData);
    printf("Text has %d lines\n", lineCount);
    
    // Sum field
    double sum = micTextFieldSum(textData, 1, '\t');
    printf("Sum of field 1: %.0f\n", sum);
    
    // Extract field
    char *field = micExtractField("sample1\t100\t200\t1.5", 2, '\t');
    if (field != NULL)
    {
        printf("Extracted field 2: %s\n", field);
        MIC_FREE(field);
    }
    
    // Head lines
    char *head = micHeadLines(textData, 2);
    if (head != NULL)
    {
        printf("First 2 lines:\n%s", head);
        MIC_FREE(head);
    }
    
    // ====================================
    // Workflow Context
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Workflow Context ---");
    
    micWorkflow wf;
    micWorkflowInit(&wf);
    micWorkflowSetWorkDir(&wf, "/tmp/workflow");
    
    micWorkflowLoadConfig(&wf, "test_config.txt");
    micWorkflowLoadSamples(&wf, "samples.txt");
    
    printf("Workflow initialized\n");
    printf("  Work directory: %s\n", wf.workDir);
    printf("  Config entries: %d\n", wf.config.count);
    printf("  Samples: %d\n", wf.samples.count);
    
    const char *workdirValue = micWorkflowConfig(&wf, "workdir");
    printf("  Config 'workdir': %s\n", workdirValue);
    
    // Expand pattern with workflow
    int wfCount = 0;
    char **wfFiles = micWorkflowExpand(&wf, "results/{sample}.txt", &wfCount);
    if (wfFiles != NULL)
    {
        printf("Workflow expanded %d files\n", wfCount);
        // Don't free wfFiles - owned by workflow
    }
    
    micWorkflowFree(&wf);
    
    // ====================================
    // Helper Functions
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Helper Functions ---");
    
    char *containerPath = micContainerPath("/containers", "fastqc.sif");
    if (containerPath != NULL)
    {
        printf("Container path: %s\n", containerPath);
        MIC_FREE(containerPath);
    }
    
    char *outputPath = micBuildPath("/output", "results/analysis.txt");
    if (outputPath != NULL)
    {
        printf("Output path: %s\n", outputPath);
        MIC_FREE(outputPath);
    }
    
    // ====================================
    // Cleanup
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Cleanup ---");
    
    micFreeSampleList(&samples);
    micFreeSampleList(&jsonSampleList);
    micConfigFree(&config);
    micConfigFree(&jsonCfg);
    
    // Delete test files
    micDeleteFile("test_config.txt");
    micDeleteFile("test_config.json");
    micDeleteFile("samples.txt");
    micDeleteFile("samples.json");
    micDeleteFile("test_data.csv");
    micDeleteFile("test_output.csv");
    micDeleteFile("gene_expression.csv");
    
    // ====================================
    // Finish
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n=== Bioinformatics Workflow Example Complete ===");
    
    return 0;
}
