# A Lightweight Bare-Metal Container Runtime for Edge AI on ARM Cortex-M (MIRA-C)

> **Kernel-less micro-isolation container runtime prototype** designed to eliminate OS/hypervisor overhead, achieving ultra-low cold start latencies and 100% hardware-enforced containment for edge-AI and high-throughput inference serving[cite: 3].

---

## Overview
Standard containerization architectures (such as Docker on Linux) rely heavily on virtualized kernel subsystems—including Namespaces, cgroups, OverlayFS, and kernel-level ELF loading—resulting in cold start latencies spanning milliseconds to seconds[cite: 3]. Furthermore, these heavy abstraction layers obstruct direct hardware acceleration access, creating a critical bottleneck for real-time AI inference.

**MIRA-C (Micro Isolation Runtime Architecture C)** bypasses kernel and hypervisor overhead entirely by leveraging **hardware-enforced Memory Protection Units (MPU)** and **CPU privilege state transitions (nPRIV)** instead of software namespaces[cite: 3].

---

## Empirical Benchmark Results
Evaluated at the Proof-of-Concept (PoC) level across continuous test runs:

| Metric / Test | Recorded Value / Status | Description |
| :--- | :--- | :--- |
| **Total Runs** | 1,000 | Continuous automated verification[cite: 5] |
| **Minimum Latency** | `11.000 µs` | Peak initialization speed[cite: 5] |
| **Mean Cold Start Latency ($T_{cold}$)** | **`14.472 µs`** | Average time decoupled from payload/image size[cite: 3, 5] |
| **Memory Protection Stress Test** | **10,000 / 10,000 Traps** | Injected out-of-bounds violation attempts[cite: 5] |
| **Containment Success Rate** | **`100.00%`** | Hardware-enforced Zero-Escape security[cite: 3, 5] |

---

## Core Architectural Pillars

1. **Hardware MPU Isolation:** Partitioning physical memory into Privileged (`Ω_sys`) and Unprivileged/Sandboxed (`Ω_app`) zones using ARM MPU registers (`MPU_RBAR`, `MPU_RASR`)[cite: 3].
2. **Privilege State Reduction:** Bypassing software context switches by manipulating CPU control registers (`nPRIV = 1`, `PSP` stack pointer transition)[cite: 3].
3. **Zero-Copy Flash Mapping:** Eliminating OverlayFS overhead entirely by mapping program instructions and model/tensor weights directly from memory headers (`mira_container_header_t`)[cite: 3].

---

## Repository Structure

```text
├── mira_c.c             # Core container runtime implementation (Pure C)
├── mira_c.h             # Zero-copy header structures & MPU definitions
├── calculator_test.c    # Functional & sandbox isolation test suite
├── test_mira            # Compiled binary test execution
└── paper.md             # Formal research notes & methodology documentation

```

---
## TEST SUIT 
```bash

gcc calculator_test.c mira_c.c -o test_mira

./test_mira
```
## Vision: Cloud-Scale Adaptation for AI Inference

While tested on resource-constrained embedded environments (ARM Cortex-M), the core principles of MIRA-C extend directly to **cloud-scale GPU fleets**:

* **Linux OCI Equivalents:** Utilizing **Landlock LSM** (for filesystem restriction without OverlayFS) and **eBPF** (for zero-escape filtering).


* **GPU Fleet Economics:** Enabling **True Scale-to-Zero** for AI inference serving platforms (e.g., serving LLMs on NVIDIA clusters) by dropping cold-start latency down to microseconds, eliminating idle power consumption and massive cluster operational expenses.



---

## License

This project is released under the [MIT License]

---

*Developed by **Asira "Marlov" Langphang** — Systems Architecture & Low-Level AI Inference Research.*

```

```
