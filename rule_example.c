/*
 * MIC Library - Rule System and Workflow Example
 * Demonstrates the new Snakemake-like rule system
 */

#define MIC_IMPLEMENTATION
#include "src/mic.h"

#include <stdio.h>

int main(void)
{
    // Initialize
    micSetTraceLogLevel(MIC_LOG_INFO);
    micTraceLog(MIC_LOG_INFO, "=== MIC Rule System and Workflow Example ===\n");
    
    // ====================================
    // Example 1: Basic Rule Creation
    // ====================================
    micTraceLog(MIC_LOG_INFO, "--- Example 1: Basic Rule Creation ---");
    
    micRule *trimRule = micRuleCreate("trim_reads");
    micRuleSetInput(trimRule, "raw/{sample}_R1.fastq.gz");
    micRuleSetInput(trimRule, "raw/{sample}_R2.fastq.gz");
    micRuleSetOutput(trimRule, "trimmed/{sample}_R1_trimmed.fastq.gz");
    micRuleSetOutput(trimRule, "trimmed/{sample}_R2_trimmed.fastq.gz");
    micRuleSetShell(trimRule, "fastp -i {input[0]} -I {input[1]} -o {output[0]} -O {output[1]}");
    micRuleSetThreads(trimRule, 4);
    micRuleSetBenchmark(trimRule, "benchmarks/{sample}.trim.txt");
    micRuleSetMessage(trimRule, "Trimming reads for {sample}");
    
    printf("Created rule: %s with %d inputs and %d outputs\n", 
           trimRule->name, trimRule->inputCount, trimRule->outputCount);
    printf("  Threads: %d\n", trimRule->threads);
    printf("  Benchmark: %s\n", trimRule->benchmark);
    
    // ====================================
    // Example 2: Workflow DAG
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 2: Workflow DAG ---");
    
    micWorkflowDAG *dag = micWorkflowDAGCreate();
    micWorkflowDAGSetWorkDir(dag, "/data/analysis");
    
    // Add trim rule to DAG
    micWorkflowDAGAddRule(dag, trimRule);
    
    // Create alignment rule
    micRule *alignRule = micRuleCreate("align_reads");
    micRuleSetInput(alignRule, "trimmed/{sample}_R1_trimmed.fastq.gz");
    micRuleSetInput(alignRule, "trimmed/{sample}_R2_trimmed.fastq.gz");
    micRuleSetOutput(alignRule, "aligned/{sample}.bam");
    micRuleSetShell(alignRule, "bowtie2 -x {config[REFERENCE]} -1 {input[0]} -2 {input[1]} | samtools view -bS - > {output[0]}");
    micRuleSetThreads(alignRule, 8);
    micRuleSetBenchmark(alignRule, "benchmarks/{sample}.align.txt");
    micWorkflowDAGAddRule(dag, alignRule);
    
    printf("Created workflow DAG with %d rules\n", dag->ruleCount);
    printf("  Working directory: %s\n", dag->workDir ? dag->workDir : "none");
    
    // Note: Not executing the workflow since the required tools/files don't exist
    // micWorkflowDAGExecute(dag, "aligned/sample1.bam");
    
    // ====================================
    // Example 3: Benchmark Tracking
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 3: Benchmark Tracking ---");
    
    micBenchmark bm;
    micBenchmarkStart(&bm, "test_rule", "sample1");
    
    // Simulate some work
    micWaitTime(100); // 100 ms
    
    micBenchmarkEnd(&bm);
    
    printf("Benchmark results:\n");
    printf("  Rule: %s\n", bm.ruleName);
    printf("  Sample: %s\n", bm.sample);
    printf("  Wall time: %.4f seconds\n", bm.wallTime);
    printf("  CPU time: %.4f seconds\n", bm.cpuTime);
    printf("  Max RSS: %.2f KB\n", bm.maxRSS);
    printf("  Timestamp: %ld\n", bm.timestamp);
    
    // Save benchmark
    const char *benchFile = "test_benchmark.csv";
    if (micBenchmarkSave(&bm, benchFile))
    {
        printf("Saved benchmark to: %s\n", benchFile);
        
        // Load and display
        if (micIsFileAvailable(benchFile))
        {
            char *content = micLoadFileText(benchFile);
            if (content != NULL)
            {
                printf("\nBenchmark file content:\n%s\n", content);
                micUnloadFileText(content);
            }
            micDeleteFile(benchFile); // Cleanup
        }
    }
    
    // ====================================
    // Example 4: Multi-Wildcard Expansion
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 4: Multi-Wildcard Expansion ---");
    
    micWildcardSet wildcard;
    wildcard.count = 3;
    strncpy(wildcard.name, "sample", sizeof(wildcard.name) - 1);
    
    wildcard.values = (char **)MIC_MALLOC(sizeof(char *) * wildcard.count);
    for (int i = 0; i < wildcard.count; i++)
    {
        wildcard.values[i] = (char *)MIC_MALLOC(64);
        sprintf(wildcard.values[i], "sample%d", i + 1);
    }
    
    int expandCount = 0;
    char **expanded = micExpandMultiWildcard("output/{sample}.txt", &wildcard, 1, &expandCount);
    
    printf("Expanded pattern to %d files:\n", expandCount);
    for (int i = 0; i < expandCount; i++)
    {
        printf("  %s\n", expanded[i]);
        MIC_FREE(expanded[i]);
    }
    if (expanded != NULL) MIC_FREE(expanded);
    
    micFreeWildcardSet(&wildcard);
    
    // ====================================
    // Example 5: Temporary File Management
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 5: Temporary File Management ---");
    
    // Create some test files
    const char *tempFile1 = "temp_test1.txt";
    const char *tempFile2 = "temp_test2.txt";
    
    micSaveFileText(tempFile1, "Temporary content 1");
    micSaveFileText(tempFile2, "Temporary content 2");
    
    // Mark as temporary
    micMarkTemporary(tempFile1);
    micMarkTemporary(tempFile2);
    
    printf("Marked %d files as temporary\n", micGetTemporaryFileCount());
    printf("  %s is temporary: %s\n", tempFile1, micIsTemporary(tempFile1) ? "yes" : "no");
    printf("  %s is temporary: %s\n", "regular.txt", micIsTemporary("regular.txt") ? "yes" : "no");
    
    // Cleanup temporary files
    micCleanupTemporaryFiles();
    printf("After cleanup: %d temporary files remain\n", micGetTemporaryFileCount());
    
    // ====================================
    // Example 6: Resource Management
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 6: Resource Management ---");
    
    micResources sysRes = micGetSystemResources();
    printf("System resources:\n");
    printf("  CPU cores: %d\n", sysRes.cpuCores);
    printf("  Memory: %d MB\n", sysRes.memoryMB);
    printf("  GPU count: %d\n", sysRes.gpuCount);
    printf("  Disk: %d MB\n", sysRes.diskMB);
    
    // Set resource requirements for a rule
    micResources required = {
        .memoryMB = 4096,
        .cpuCores = 4,
        .gpuCount = 0,
        .diskMB = 10000
    };
    
    micRuleSetResources(trimRule, &required);
    
    bool available = micCheckResourcesAvailable(&required);
    printf("\nRequired resources available: %s\n", available ? "yes" : "no");
    
    // ====================================
    // Example 7: Rule Copying
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 7: Rule Copying ---");
    
    micRule *trimRuleCopy = micRuleCopy(trimRule, "trim_reads_alternative");
    micRuleOverrideShell(trimRuleCopy, "trimmomatic PE {input[0]} {input[1]} {output[0]} {output[1]}");
    
    printf("Created copy of rule '%s' as '%s'\n", trimRule->name, trimRuleCopy->name);
    printf("  Original shell: %s\n", trimRule->shell);
    printf("  Copy shell: %s\n", trimRuleCopy->shell);
    
    micRuleFree(trimRuleCopy);
    
    // ====================================
    // Example 8: Rule with Temporary Output
    // ====================================
    micTraceLog(MIC_LOG_INFO, "\n--- Example 8: Rule with Temporary Output ---");
    
    micRule *tempRule = micRuleCreate("temp_processing");
    micRuleSetInput(tempRule, "input.txt");
    micRuleSetOutput(tempRule, "temp_output.txt");
    micRuleSetShell(tempRule, "process input.txt > temp_output.txt");
    micRuleSetTemporary(tempRule, true);
    
    printf("Created rule '%s' with temporary output: %s\n", 
           tempRule->name, tempRule->temporary ? "yes" : "no");
    
    micRuleFree(tempRule);
    
    // ====================================
    // Cleanup
    // ====================================
    micWorkflowDAGFree(dag);
    
    micTraceLog(MIC_LOG_INFO, "\n=== Example completed successfully ===");
    
    return 0;
}
