# CYBERSENTINEL
## Distributed Network Anomaly Detection and Threat Analytics Engine

**Module:** SIT315 — Distributed and Hybrid Programming  
**Task:** M4.T1D — Module 4 Project  
**Student:** Paarthipa Harish  
**Project Type:** Defensive Cybersecurity Analytics

---

## 1. Project Overview

CYBERSENTINEL is a standalone defensive cybersecurity analytics engine designed to analyse large-scale synthetic network-flow data and identify potentially suspicious network behaviour.

The project combines distributed-memory and shared-memory parallelism using:

- MPI for distributed process-level workload decomposition
- OpenMP for thread-level parallelism
- Adaptive master-worker scheduling
- Two-stage threat analysis
- Threat scoring and severity classification
- Performance and throughput measurement
- Per-rank workload and timing analysis
- Load-balance measurement
- Large-scale stress testing

The system operates entirely on controlled synthetic network-flow datasets and does not perform live network monitoring or offensive security operations.

---

## 2. Main Features

CYBERSENTINEL provides four execution modes.

### Sequential Mode

A single-process baseline used for comparison.

```bash
./bin/cybersentinel \
--mode seq \
--input data/network_10k.csv
```

### MPI Mode

Distributes network-flow processing across multiple MPI processes.

```bash
mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode mpi \
--input data/network_10k.csv
```

### Hybrid MPI/OpenMP Mode

Combines MPI process-level distribution with OpenMP thread-level parallelism.

```bash
OMP_NUM_THREADS=4 mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode hybrid \
--input data/network_10k.csv
```

### Adaptive Mode

Uses a master-worker scheduling model where work is divided into chunks and dynamically assigned to worker processes.

```bash
OMP_NUM_THREADS=4 mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode adaptive \
--input data/network_10k.csv \
--chunks 8
```

---

## 3. System Architecture

The overall processing pipeline is:

```text
Synthetic Network Dataset
          |
          v
    Workload Chunking
          |
          v
    MPI Distribution
          |
          v
   MPI Worker Processes
          |
          v
    OpenMP Parallelism
          |
          v
 Stage 1: Candidate Filtering
          |
          v
 Stage 2: Threat Analysis
          |
          v
 Threat Scoring & Classification
          |
          v
   MPI Result Aggregation
          |
          v
 Global Security Summary
```

The adaptive execution mode additionally records worker timing and workload information to measure load balance.

---

## 4. Threat Analysis

The synthetic network-flow dataset contains heterogeneous traffic patterns including:

- NORMAL
- SCANNING
- BURST
- HIGH_BYTES
- FAILED_CONNECTION
- MIXED

CYBERSENTINEL performs two-stage analysis.

### Stage 1 — Candidate Filtering

Network records are rapidly examined to identify records that require further analysis.

### Stage 2 — Threat Analysis

Suspicious candidates undergo additional analysis based on characteristics such as:

- Port scanning behaviour
- Traffic bursts
- Unusually high byte counts
- Failed connections
- Destination diversity

The system calculates a threat score and assigns a severity classification.

---

## 5. Project Structure

```text
CYBERSENTINEL/
│
├── include/
│   ├── analyzer.hpp
│   ├── dataset.hpp
│   ├── mpi_engine.hpp
│   ├── network_record.hpp
│   ├── omp_engine.hpp
│   ├── performance.hpp
│   ├── scheduler.hpp
│   └── threat_detector.hpp
│
├── src/
│   ├── analyzer.cpp
│   ├── dataset.cpp
│   ├── main.cpp
│   ├── mpi_engine.cpp
│   ├── network_record.cpp
│   ├── omp_engine.cpp
│   ├── performance.cpp
│   ├── scheduler.cpp
│   └── threat_detector.cpp
│
├── scripts/
│   └── generate_dataset.py
│
├── phase0_tests/
│   └── Environment verification files
│
├── data/
│   └── Synthetic datasets
│
├── results/
│   └── Experimental results
│
├── screenshots/
│   └── Project evidence screenshots
│
└── README.md
```

Large generated datasets are not required to reproduce the source implementation and can be regenerated using the dataset-generation script.

---

## 6. Requirements

The project was developed and tested on an Apple-silicon Mac environment.

Required software:

- macOS
- C++17 compiler
- Open MPI
- OpenMP / Homebrew `libomp`
- Python 3

The project uses:

- `mpic++`
- MPI
- OpenMP
- Python 3
- C++17

---

## 7. Compilation

Run the following commands from the project root directory:

```bash
mkdir -p bin

export OMP_INC="$(brew --prefix libomp)/include"
export OMP_LIB="$(brew --prefix libomp)/lib"

mpic++ -std=c++17 -O2 \
-Iinclude \
-Xpreprocessor -fopenmp \
-I"$(brew --prefix libomp)/include" \
-L"$(brew --prefix libomp)/lib" \
-lomp \
src/main.cpp \
src/network_record.cpp \
src/dataset.cpp \
src/threat_detector.cpp \
src/analyzer.cpp \
src/omp_engine.cpp \
src/mpi_engine.cpp \
src/scheduler.cpp \
src/performance.cpp \
-o bin/cybersentinel
```

Verify the executable:

```bash
ls -lh bin/cybersentinel
```

---

## 8. Dataset Generation

Synthetic datasets can be generated using the included Python script.

### Generate 10,000 Records

```bash
python3 scripts/generate_dataset.py \
--records 10000 \
--output data/network_10k.csv \
--seed 42
```

### Generate 100,000 Records

```bash
python3 scripts/generate_dataset.py \
--records 100000 \
--output data/network_100k.csv \
--seed 42
```

### Generate 1 Million Records

```bash
python3 scripts/generate_dataset.py \
--records 1000000 \
--output data/network_1m.csv \
--seed 42
```

### Generate 10 Million Records

```bash
python3 scripts/generate_dataset.py \
--records 10000000 \
--output data/network_10m.csv \
--seed 42
```

The fixed seed provides consistent synthetic workload generation for repeatable experiments.

---

## 9. Running CYBERSENTINEL

### Sequential Execution

```bash
./bin/cybersentinel \
--mode seq \
--input data/network_10k.csv
```

### MPI Execution

```bash
mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode mpi \
--input data/network_10k.csv
```

### Hybrid MPI/OpenMP Execution

```bash
OMP_NUM_THREADS=4 mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode hybrid \
--input data/network_10k.csv
```

### Adaptive Execution

```bash
OMP_NUM_THREADS=4 mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode adaptive \
--input data/network_10k.csv \
--chunks 8
```

---

## 10. Adaptive 10-Million-Record Experiment

The principal adaptive experiment uses four MPI processes, with rank 0 acting as the scheduler and the remaining ranks acting as workers.

```bash
OMP_NUM_THREADS=4 mpirun --oversubscribe -np 4 \
./bin/cybersentinel \
--mode adaptive \
--input data/network_10m.csv \
--chunks 8
```

The experiment uses:

```text
MPI processes:          4
Scheduler rank:         0
Worker ranks:           3
OpenMP threads/worker:  4
Total chunks:           24
Chunks/worker:          8
```

The adaptive output reports:

- Records processed by each worker
- Worker execution time
- Maximum worker time
- Minimum worker time
- Average worker time
- Load imbalance
- Overall throughput

---

## 11. Performance Evaluation

The main performance evaluation uses increasing workload sizes:

```text
100,000 records
1,000,000 records
10,000,000 records
```

An additional 50-million-record workload was used for project-processing stress testing.

The following execution strategies are compared:

```text
Sequential
MPI ×4
Hybrid MPI/OpenMP 4×4
Adaptive Hybrid
```

The main performance metrics are:

- Execution time
- Throughput
- Speedup
- Worker execution time
- Records processed per worker
- Load imbalance

Speedup is calculated as:

```text
Speedup = Sequential Execution Time / Parallel Execution Time
```

---

## 12. Load-Balance Evaluation

The adaptive scheduler uses a master-worker model.

Rank 0 maintains the workload distribution while the worker ranks process assigned chunks.

Workers can receive additional chunks as they complete their current work. This allows the system to respond to differences in computational workload between chunks.

The project records per-rank:

- Number of records processed
- Number of chunks processed
- Execution time

These measurements are used to calculate the observed load imbalance.

---

## 13. Stress Testing

CYBERSENTINEL was tested beyond the main evaluation workloads.

A 50-million-record workload was successfully generated and processed sequentially.

A larger 500-million-record dataset-generation attempt was also performed. The generation process was manually interrupted after:

```text
Elapsed time: 10 minutes 33.04 seconds
Records generated: 302,508,528
Records requested: 500,000,000
Status: INTERRUPTED
```

The stress experiment was documented as a practical workload-boundary test rather than being reported as a completed 500-million-record experiment.

---

## 14. Reproducibility

The project uses controlled synthetic network-flow data and fixed random seeds.

For example:

```bash
python3 scripts/generate_dataset.py \
--records 10000000 \
--output data/network_10m.csv \
--seed 42
```

The generated dataset can then be analysed using the execution commands provided in this README.

Execution times can vary depending on system load, operating-system scheduling and available resources.

---

## 15. Source Code Organisation

The implementation is separated into modular components:

```text
network_record.cpp   → Network-flow record representation
dataset.cpp          → Dataset handling
threat_detector.cpp  → Candidate detection, scoring and classification
analyzer.cpp         → Analysis coordination
omp_engine.cpp       → OpenMP processing
mpi_engine.cpp       → MPI distributed processing
scheduler.cpp        → Adaptive master-worker scheduling
performance.cpp      → Timing and performance measurement
main.cpp             → Command-line execution and mode selection
```

The corresponding header files are located in the `include/` directory.

---

## 16. Project Contribution

CYBERSENTINEL extends a basic parallel-processing implementation through:

- Hybrid MPI/OpenMP execution
- Adaptive master-worker scheduling
- Heterogeneous synthetic cybersecurity workloads
- Two-stage threat analysis
- Threat scoring and severity classification
- Per-rank workload instrumentation
- Explicit load-balance measurement
- Multiple increasing workload sizes
- Large-scale stress testing
- Comparative sequential, MPI, hybrid and adaptive evaluation

The project is implemented as a standalone defensive cybersecurity analytics application using controlled synthetic data.

---

## 17. Important Notes

The `data/` directory may contain large generated datasets. These datasets can be regenerated using the included dataset-generation script.

The `bin/` directory contains compiled binaries and can be recreated using the compilation command in this README.

For assessment reproduction, the primary required files are:

- Source code
- Header files
- Dataset-generation script
- Phase 0 verification files
- README

Large generated datasets and compiled binaries do not need to be included in the source-code submission unless specifically requested.

---

## 18. Submission Contents

The project code package contains the source implementation and supporting files required to reproduce CYBERSENTINEL.

The accompanying project report documents:

- Project scope
- System architecture
- Implementation
- Experimental methodology
- Performance results
- Load-balance evaluation
- Stress testing
- Exact compilation and execution commands
- Experimental evidence
- Results and discussion
- Project contribution

The accompanying video demonstrates the working system and its main experimental results.

---

## 19. Author

**Paarthipa Harish**

**SIT315 — Distributed and Hybrid Programming**

**Module 4 Project — Task M4.T1D**

**CYBERSENTINEL — Distributed Network Anomaly Detection and Threat Analytics Engine**
