# Bioinformatics Workflow Support

This document describes the bioinformatics workflow features added to the `mic` library, designed to support building workflows similar to Snakemake pipelines in C.

## Overview

The mic library now includes comprehensive support for bioinformatics workflow automation, including:

- Configuration file management (simple key=value and JSON)
- Sample list management
- File pattern expansion with wildcards
- CSV file operations
- Gzip/compressed file handling
- AWK-like text processing
- Workflow context management
- Helper path functions

## Features

### 1. Configuration Management

Load and manage pipeline configuration from files:

```c
micConfig config;
config.count = 0;

// Load from simple key=value file
micLoadConfig(&config, "pipeline.conf");

// Load from JSON file
micLoadConfigJSON(&config, "config.json");

// Get configuration values
const char *threads = micConfigGet(&config, "threads");
const char *quality = micConfigGetDefault(&config, "quality_threshold", "30");

// Set configuration values
micConfigSet(&config, "output_dir", "/results");

// Free resources
micConfigFree(&config);
```

Configuration file format (simple):
```
# Pipeline configuration
workdir=/tmp/pipeline
threads=4
quality_threshold=30
output_format=fastq
```

Configuration file format (JSON):
```json
{
  "workdir": "/tmp/pipeline",
  "threads": "4",
  "quality_threshold": "30",
  "output_format": "fastq"
}
```

### 2. Sample List Management

Manage lists of samples for batch processing:

```c
micSampleList samples;
samples.samples = NULL;
samples.count = 0;

// Load from text file (one sample per line)
micLoadSamplesList(&samples, "samples.txt");

// Load from JSON array
micLoadSamplesJSON(&samples, "samples.json");

// Create from array
const char *sampleNames[] = {"sample1", "sample2", "sample3"};
micCreateSampleList(&samples, sampleNames, 3);

// Access samples
for (int i = 0; i < samples.count; i++) {
    const char *sample = micGetSample(&samples, i);
    printf("Processing %s\n", sample);
}

// Free resources
micFreeSampleList(&samples);
```

Sample list file format (text):
```
sample1
sample2
sample3
control1
```

Sample list file format (JSON):
```json
["sample1", "sample2", "sample3", "control1"]
```

### 3. File Pattern Expansion

Expand file patterns with wildcards (like Snakemake's `expand()` function):

```c
micSampleList samples;
// ... load samples ...

int count;
char **files = micExpand("output/{sample}_R1.fastq", &samples, &count);

for (int i = 0; i < count; i++) {
    printf("%s\n", files[i]);
    // output/sample1_R1.fastq
    // output/sample2_R1.fastq
    // etc.
}

micFreeExpandedFiles(files, count);
```

Extract wildcard values from filenames:

```c
const char *pattern = "metrics/{sample}_R1.stats";
const char *filename = "metrics/sample1_R1.stats";

char *sample = micExtractWildcard(pattern, filename, "sample");
printf("Sample: %s\n", sample);  // "sample1"
MIC_FREE(sample);
```

Replace wildcards in patterns:

```c
char *expanded = micReplaceWildcard("output/{sample}.txt", "sample", "test123");
printf("%s\n", expanded);  // "output/test123.txt"
MIC_FREE(expanded);
```

Check if files exist and wait for them:

```c
if (micAllFilesExist(files, count)) {
    printf("All files ready!\n");
}

// Wait up to 60 seconds for all files
if (micWaitForAllFiles(files, count, 60000)) {
    printf("All files arrived\n");
}
```

### 4. CSV File Operations

Complete CSV reading, writing, and manipulation:

```c
// Load CSV file
micCSVFile *csv = micLoadCSV("data.csv", ',', true);  // true = has header

if (csv != NULL) {
    printf("Loaded %d rows, %d columns\n", csv->rowCount, csv->headerCount);
    
    // Access by indices
    const char *value = micCSVGetValue(csv, 0, 1);
    
    // Access by header name
    const char *count = micCSVGetValueByName(csv, 0, "ReadCount");
    
    // Sum column values
    double totalReads = micCSVSumColumn(csv, 1);
    
    // Count matching rows
    int controls = micCSVCountWhere(csv, 0, "control");
    
    // Save CSV
    micSaveCSV("output.csv", csv);
    
    // Free resources
    micFreeCSV(csv);
}
```

Create CSV from scratch:

```c
const char *headers[] = {"Gene", "Expression", "Pvalue"};
micCSVFile *csv = micCSVCreate(headers, 3, ',');

const char *row1[] = {"GeneA", "5.2", "0.001"};
const char *row2[] = {"GeneB", "3.8", "0.05"};

micCSVAddRow(csv, row1, 3);
micCSVAddRow(csv, row2, 3);

micSaveCSV("results.csv", csv);
micFreeCSV(csv);
```

### 5. Gzip/Compressed File Operations

Work with gzipped files (requires zlib, enabled with `SUPPORT_COMPRESSION_API`):

```c
// Read gzipped file
unsigned int size;
unsigned char *data = micLoadGzipFile("reads.fastq.gz", &size);

// Read gzipped text file
char *text = micLoadGzipFileText("data.txt.gz");

// Save to gzipped file
micSaveGzipFile("output.gz", data, size);
micSaveGzipFileText("output.txt.gz", "text content");

// Concatenate gzipped files
const char *files[] = {"part1.gz", "part2.gz", "part3.gz"};
micConcatGzipFiles(files, 3, "merged.gz");
```

### 6. AWK-like Text Processing

Process text data similar to AWK commands:

```c
const char *data = "sample1\t100\t200\t1.5\n"
                   "sample2\t150\t180\t2.1\n"
                   "sample3\t120\t220\t1.8\n";

// Count lines
int lines = micCountLines(data);

// Sum values in a field
double sum = micTextFieldSum(data, 1, '\t');  // Sum field 1

// Extract specific field
char *field = micExtractField("sample1\t100\t200", 2, '\t');

// Filter lines
char *filtered = micFilterLines(data, 0, "sample1", '\t');

// Get first/last n lines
char *head = micHeadLines(data, 2);
char *tail = micTailLines(data, 2);

// Sort by field
char *sorted = micSortByField(data, 1, '\t', false, true);  // numeric ascending
```

### 7. Workflow Context

High-level workflow management:

```c
micWorkflow wf;
micWorkflowInit(&wf);

// Set working directory
micWorkflowSetWorkDir(&wf, "/tmp/workflow");

// Load configuration
micWorkflowLoadConfig(&wf, "config.txt");  // or config.json

// Load samples
micWorkflowLoadSamples(&wf, "samples.txt");  // or samples.json

// Get config values
const char *threads = micWorkflowConfig(&wf, "threads");

// Expand patterns
int count;
char **files = micWorkflowExpand(&wf, "results/{sample}.txt", &count);

// Process workflow...

// Free resources
micWorkflowFree(&wf);
```

### 8. Helper Functions

Utility functions for path management:

```c
// Build container path
char *container = micContainerPath("/containers", "fastqc.sif");
// Result: "/containers/fastqc.sif"

// Build output path
char *output = micBuildPath("/output", "results/analysis.txt");
// Result: "/output/results/analysis.txt"

// Ensure output directory exists
micEnsureOutputDir("/output/results/analysis.txt");
// Creates "/output/results" if it doesn't exist

// Remember to free allocated strings
MIC_FREE(container);
MIC_FREE(output);
```

## Example Workflow

Here's a complete bioinformatics workflow example:

```c
#define MIC_IMPLEMENTATION
#include "src/mic.h"

int main(void) {
    micSetTraceLogLevel(MIC_LOG_INFO);
    
    // Initialize workflow
    micWorkflow wf;
    micWorkflowInit(&wf);
    micWorkflowSetWorkDir(&wf, "/data/phage_display");
    
    // Load configuration and samples
    micWorkflowLoadConfig(&wf, "config.txt");
    micWorkflowLoadSamples(&wf, "samples.txt");
    
    // Get configuration
    const char *threads = micWorkflowConfig(&wf, "threads");
    const char *quality = micWorkflowConfig(&wf, "quality_threshold");
    
    micTraceLog(MIC_LOG_INFO, "Processing %d samples with %s threads", 
                wf.samples.count, threads);
    
    // Expand file patterns
    int count;
    char **inputFiles = micWorkflowExpand(&wf, "input/{sample}_R1.fastq.gz", &count);
    
    // Process each sample
    for (int i = 0; i < wf.samples.count; i++) {
        const char *sample = micGetSample(&wf.samples, i);
        
        micTraceLog(MIC_LOG_INFO, "Processing sample: %s", sample);
        
        // Build output filenames
        char *outputFile = micReplaceWildcard("output/{sample}_processed.fastq", 
                                              "sample", sample);
        
        // Ensure output directory exists
        micEnsureOutputDir(outputFile);
        
        // Process sample (example)
        const char *cmd = micStringFormat(
            "fastqc -t %s -q %s %s -o %s",
            threads, quality, inputFiles[i], outputFile
        );
        
        micExecuteCommand(cmd);
        
        MIC_FREE(outputFile);
    }
    
    // Load and process CSV results
    micCSVFile *csv = micLoadCSV("results.csv", ',', true);
    if (csv != NULL) {
        double totalReads = micCSVSumColumn(csv, 1);
        micTraceLog(MIC_LOG_INFO, "Total reads: %.0f", totalReads);
        micFreeCSV(csv);
    }
    
    // Cleanup
    micFreeExpandedFiles(inputFiles, count);
    micWorkflowFree(&wf);
    
    return 0;
}
```

Compile with:
```bash
gcc -o workflow workflow.c -lm -lz
```

## Data Structures

### micConfig
Configuration storage with key-value pairs (max 256 entries).

### micSampleList
List of sample names for batch processing.

### micCSVFile
Complete CSV file representation with headers and data rows.

### micWorkflow
High-level workflow context combining config, samples, and working directory.

## Constants

- `MIC_MAX_SAMPLES` (1000) - Maximum samples in a sample list
- `MIC_MAX_PATTERN_LENGTH` (512) - Maximum length of file patterns
- `MIC_MAX_CONFIG_ENTRIES` (256) - Maximum configuration entries
- `MIC_MAX_CSV_FIELDS` (256) - Maximum fields per CSV row
- `MIC_MAX_CSV_ROWS` (100000) - Maximum rows in a CSV file

## Compilation Notes

### Basic Compilation
```bash
gcc -o myworkflow myworkflow.c
```

### With Compression Support
To enable gzip file operations, define `SUPPORT_COMPRESSION_API` and link with zlib:
```bash
gcc -o myworkflow myworkflow.c -lz
```

### With Math Functions
Some functions (CSV sum, text processing) may require math library:
```bash
gcc -o myworkflow myworkflow.c -lm
```

### Complete
```bash
gcc -o myworkflow myworkflow.c -lm -lz
```

## Error Handling

Most functions return:
- `-1` or `NULL` on error
- `0` or positive value on success
- `false`/`true` for boolean operations

Always check return values and use the logging system:

```c
micSetTraceLogLevel(MIC_LOG_INFO);

if (micLoadConfig(&config, "config.txt") < 0) {
    micTraceLog(MIC_LOG_ERROR, "Failed to load configuration");
    return 1;
}
```

## Memory Management

Functions that allocate memory and return strings or arrays need to be freed:

```c
char **files = micExpand(pattern, &samples, &count);
// ... use files ...
micFreeExpandedFiles(files, count);

char *replaced = micReplaceWildcard(pattern, "sample", "test");
// ... use replaced ...
MIC_FREE(replaced);

micCSVFile *csv = micLoadCSV("data.csv", ',', true);
// ... use csv ...
micFreeCSV(csv);
```

## See Also

- [MIC Library README](README.md) - Main documentation
- [HPC Features](HPC_FEATURES.md) - SLURM and container support
- [Advanced Features](ADVANCED_FEATURES.md) - Pipeline features
- `bioinformatics_example.c` - Complete working example
- `rule_example.c` - Rule system example

---

## Rule System and Workflow Management (Snakemake-like)

The mic library includes a comprehensive rule-based workflow system similar to Snakemake, allowing you to define modular, reusable pipeline steps with automatic dependency resolution.

### 1. Rule Definition

Create rules to define workflow steps:

```c
// Create a rule
micRule *trimRule = micRuleCreate("trim_reads");

// Set inputs and outputs with wildcards
micRuleSetInput(trimRule, "raw/{sample}_R1.fastq.gz");
micRuleSetInput(trimRule, "raw/{sample}_R2.fastq.gz");
micRuleSetOutput(trimRule, "trimmed/{sample}_R1_trimmed.fastq.gz");
micRuleSetOutput(trimRule, "trimmed/{sample}_R2_trimmed.fastq.gz");

// Set shell command
micRuleSetShell(trimRule, "fastp -i {input[0]} -I {input[1]} -o {output[0]} -O {output[1]}");

// Set additional properties
micRuleSetThreads(trimRule, 4);
micRuleSetBenchmark(trimRule, "benchmarks/{sample}.trim.txt");
micRuleSetMessage(trimRule, "Trimming reads for {sample}");
micRuleSetLog(trimRule, "logs/{sample}.trim.log");

// Execute rule (with wildcards)
const char *wildcards[] = {"sample1"};
micRuleExecute(trimRule, wildcards, 1);

// Free rule
micRuleFree(trimRule);
```

### 2. Workflow DAG (Directed Acyclic Graph)

Build workflows with multiple rules and automatic dependency resolution:

```c
// Create workflow DAG
micWorkflowDAG *dag = micWorkflowDAGCreate();
micWorkflowDAGSetWorkDir(dag, "/data/analysis");

// Add rules to DAG
micWorkflowDAGAddRule(dag, trimRule);
micWorkflowDAGAddRule(dag, alignRule);
micWorkflowDAGAddRule(dag, callVariantsRule);

// Execute workflow targeting final output
micWorkflowDAGExecute(dag, "variants/sample1.vcf");

// Free DAG (also frees all rules)
micWorkflowDAGFree(dag);
```

### 3. Benchmark Tracking

Track execution time and resource usage for each rule:

```c
micBenchmark bm;

// Start tracking
micBenchmarkStart(&bm, "align_reads", "sample1");

// ... execute rule ...

// End tracking
micBenchmarkEnd(&bm);

// Access benchmark data
printf("Wall time: %.2f seconds\n", bm.wallTime);
printf("CPU time: %.2f seconds\n", bm.cpuTime);
printf("Max RSS: %.2f KB\n", bm.maxRSS);

// Save benchmark to CSV file
micBenchmarkSave(&bm, "benchmarks/sample1.align.csv");

// Load all benchmarks from directory
micCSVFile *benchmarks = micBenchmarkLoadAll("benchmarks");
// ... analyze benchmarks ...
micFreeCSV(benchmarks);
```

### 4. Multi-Wildcard Expansion

Expand patterns with multiple wildcards:

```c
// Define wildcard sets
micWildcardSet sampleWildcard;
sampleWildcard.count = 3;
strncpy(sampleWildcard.name, "sample", sizeof(sampleWildcard.name) - 1);
sampleWildcard.values = (char **)MIC_MALLOC(sizeof(char *) * sampleWildcard.count);
sampleWildcard.values[0] = "sample1";
sampleWildcard.values[1] = "sample2";
sampleWildcard.values[2] = "sample3";

// Expand pattern
int count;
char **files = micExpandMultiWildcard("output/{sample}.bam", &sampleWildcard, 1, &count);

for (int i = 0; i < count; i++) {
    printf("%s\n", files[i]);  // output/sample1.bam, output/sample2.bam, etc.
    MIC_FREE(files[i]);
}
MIC_FREE(files);

micFreeWildcardSet(&sampleWildcard);
```

### 5. Temporary File Management

Mark files as temporary for automatic cleanup:

```c
// Mark file as temporary
micMarkTemporary("intermediate/temp_data.txt");

// Check if file is temporary
if (micIsTemporary("intermediate/temp_data.txt")) {
    printf("File is temporary\n");
}

// Get count of temporary files
int tempCount = micGetTemporaryFileCount();

// Cleanup all temporary files at workflow end
micCleanupTemporaryFiles();
```

### 6. Resource Management

Specify and check resource requirements:

```c
// Get system resources
micResources sysRes = micGetSystemResources();
printf("CPU cores: %d\n", sysRes.cpuCores);
printf("Memory: %d MB\n", sysRes.memoryMB);

// Set resource requirements for a rule
micResources required = {
    .memoryMB = 4096,
    .cpuCores = 4,
    .gpuCount = 0,
    .diskMB = 10000
};

micRuleSetResources(alignRule, &required);

// Check if resources are available
if (micCheckResourcesAvailable(&required)) {
    micRuleExecute(alignRule, NULL, 0);
} else {
    micTraceLog(MIC_LOG_ERROR, "Insufficient resources");
}
```

### 7. Rule Inheritance and Copying

Create rule variants by copying and modifying:

```c
// Copy rule with new name
micRule *trimAltRule = micRuleCopy(trimRule, "trim_reads_alternative");

// Override specific properties
micRuleOverrideShell(trimAltRule, "trimmomatic PE {input[0]} {input[1]} {output[0]} {output[1]}");
micRuleOverrideInput(trimAltRule, 0, "raw_alt/{sample}_R1.fastq.gz");

// Execute with different parameters
micRuleExecute(trimAltRule, wildcards, 1);

micRuleFree(trimAltRule);
```

### 8. Container Support

Execute rules in containers (Singularity/Docker):

```c
micRule *alignRule = micRuleCreate("align_reads");
micRuleSetInput(alignRule, "trimmed/{sample}_R1.fastq.gz");
micRuleSetOutput(alignRule, "aligned/{sample}.bam");
micRuleSetShell(alignRule, "bwa mem ref.fa {input[0]} | samtools view -b > {output[0]}");

// Set container image
micRuleSetContainer(alignRule, "/containers/bwa_samtools.sif");

// Rule will execute in container automatically
micRuleExecute(alignRule, wildcards, 1);
```

### 9. Dynamic Input Functions

Use function pointers for dynamic input determination:

```c
// Define input function
char **getAlignmentInputs(const char *sample, void *userData, int *count) {
    // Dynamically determine inputs based on sample
    *count = 2;
    char **inputs = (char **)MIC_MALLOC(sizeof(char *) * 2);
    inputs[0] = (char *)MIC_MALLOC(256);
    inputs[1] = (char *)MIC_MALLOC(256);
    sprintf(inputs[0], "trimmed/%s_R1.fastq.gz", sample);
    sprintf(inputs[1], "trimmed/%s_R2.fastq.gz", sample);
    return inputs;
}

// Set input function for rule
micRuleSetInputFunction(alignRule, getAlignmentInputs, NULL);
```

### Complete Workflow Example

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
    
    // Define alignment rule
    micRule *alignRule = micRuleCreate("align_reads");
    micRuleSetInput(alignRule, "trimmed/{sample}_R1_trimmed.fastq.gz");
    micRuleSetInput(alignRule, "trimmed/{sample}_R2_trimmed.fastq.gz");
    micRuleSetOutput(alignRule, "aligned/{sample}.bam");
    micRuleSetShell(alignRule, "bowtie2 -x ref -1 {input[0]} -2 {input[1]} | samtools view -bS - > {output[0]}");
    micRuleSetThreads(alignRule, 8);
    micRuleSetBenchmark(alignRule, "benchmarks/{sample}.align.txt");
    micWorkflowDAGAddRule(dag, alignRule);
    
    // Execute workflow
    micWorkflowDAGExecute(dag, "aligned/sample1.bam");
    
    // Cleanup
    micWorkflowDAGFree(dag);
    
    return 0;
}
```

### Rule System Memory Management

Rules and workflows must be properly freed:

```c
// Rules added to DAG are freed when DAG is freed
micWorkflowDAGFree(dag);  // Frees all rules in DAG

// Standalone rules must be freed manually
micRule *rule = micRuleCreate("test");
// ... use rule ...
micRuleFree(rule);

// Wildcard sets must be freed
micFreeWildcardSet(&wildcardSet);

// Temporary files are automatically deleted on cleanup
micCleanupTemporaryFiles();
```
