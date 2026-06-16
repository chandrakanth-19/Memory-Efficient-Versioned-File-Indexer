# CS253 Assignment 1 - Memory-Efficient Versioned File Indexer
**Name:** Katroth Chandrakanth
**Roll Number:** 240531

## Overview

This project implements a **memory-efficient versioned file indexer** in C++ that processes large text files incrementally using a fixed-size buffer. The system constructs a word-level index without loading the entire file into memory and supports multiple versions of indexed datasets within a single execution.

The implementation emphasizes object-oriented design principles, efficient memory management, and scalable processing techniques.

---

## Features

* Incremental processing of large text files using a fixed-size buffer.
* Case-insensitive word indexing.
* Support for multiple file versions.
* Word frequency lookup for a specific version.
* Frequency difference comparison between two versions.
* Top-K most frequent words retrieval.
* Correct handling of words split across buffer boundaries.
* Memory usage independent of input file size.

---

## Object-Oriented Design

The project follows an object-oriented architecture consisting of the following major components:

### Buffered File Reader

Responsible for reading files incrementally using a fixed-size buffer.

### Tokenizer

Extracts alphanumeric words from the incoming stream while handling tokens spanning buffer boundaries.

### Versioned Index Manager

Maintains separate word-frequency indices for multiple versions.

### Query Processor

Processes user queries and generates the required outputs.

---

## C++ Concepts Demonstrated

* Classes and Objects
* Abstract Base Classes
* Inheritance
* Runtime Polymorphism using Virtual Functions
* Function Overloading
* Exception Handling (`try`, `throw`, `catch`)
* User-Defined Templates
* STL Containers and Algorithms

---

## Memory Management Strategy

The program avoids loading the complete dataset into memory. Instead, it processes the file chunk by chunk using a fixed-size buffer ranging from **256 KB to 1024 KB**.

Memory consumption grows only with the number of unique words encountered rather than the overall file size.


## How to Compile

```bash
g++ c++_source_code.cpp -o analyzer
```


## How to Run

### Word Query
```bash
.\analyzer --file test_logs.txt --version v1 --buffer 512 --query word --word error
```

### Top-K Query
```bash
.\analyzer --file test_logs.txt --version v1 --buffer 512 --query top --top 10
```

### Diff Query
```bash
.\analyzer --file1 test_logs.txt --version1 v1 --file2 verbose_logs.txt --version2 v2 --buffer 512 --query diff --word error
```

## Program Output

The program displays:
- Version name(s)
- Query result
- Allocated buffer size (KB)
- Total execution time (seconds)

Refer to `Execution_screenshot.png` included in the submission package.


## Command Line Arguments

| Argument | Description |
|---|---|
| `--file` | Input file path (word/top query) |
| `--file1` | First input file (diff query) |
| `--file2` | Second input file (diff query) |
| `--version` | Version name (word/top query) |
| `--version1` | First version name (diff query) |
| `--version2` | Second version name (diff query) |
| `--buffer` | Buffer size in KB (256 to 1024) |
| `--query` | Query type: word, top, diff |
| `--word` | Word to search (word/diff query) |
| `--top` | Number of top results (top query) |

## Classes Overview
- **bufferread** — reads file in fixed size chunks, never loads entire file
- **Tokenizer** — extracts lowercase alphanumeric tokens from a chunk
- **versionedindex** — stores word frequency maps per version
- **query** — base class for all query types
- **wordquery** — returns frequency of a word in a version
- **topquery** — returns topK most frequent words
- **diffquery** — returns frequency difference between two versions


## Notes
- Buffer size must be between 256 and 1024 KB
- Words split across buffer boundaries are handled correctly using a carry string
- Word matching is case insensitive
- Memory usage grows only with number of unique words, not file size
