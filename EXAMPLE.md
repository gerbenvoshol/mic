# MIC Library Example

This example demonstrates the functionality of the mic (make-it-c) library.

## Compilation

```bash
gcc -o example example.c -Wall
```

## Running

```bash
./example
```

## Features Demonstrated

### 1. Environment Information
- Operating system detection
- Platform architecture detection
- Machine name retrieval

### 2. String Operations
- String case conversion (upper, lower, Pascal case)
- String comparison and searching
- String formatting and manipulation
- String joining and splitting
- Substring extraction

### 3. File System Operations
- File creation, reading, and writing
- File information (size, extension, name)
- File copying and deletion
- Directory creation and deletion
- Working directory operations

### 4. Random Number Generation
- Setting random seed
- Generating random values in range

### 5. Timing Operations
- Timestamp generation
- Timer initialization
- Time measurement
- Wait/sleep operations

### 6. Storage
- Saving and loading integer values to persistent storage

### 7. Process Tracking
- Process and step logging for pipeline execution

## Notes

- All functions are implemented and tested on Linux
- Windows compatibility is maintained where possible
- Compression functions require external libraries and are not implemented
- Some advanced features (recursive directory operations) have basic implementations
