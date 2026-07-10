# 3D-Trading-Platform
A modular, native 3D financial trading platform engine written in C++20 and Vulkan 1.3

# Project Title 🚧 (Work in Progress)

A concise description of what this project will eventually be once completed. 

> **Current Status:** This project is under active development. The core systems are being built, and it is not yet ready for production or general use.

## 🗺️ Development Roadmap

### Phase 1: Core Foundation
- [x] Set up project architecture and folder structures
- [x] Configure build system (CMake/Scripts)
- [x] Implement memory logging/tracking system

### Phase 2: Core Features (Current Focus)
- [x] Implement the data processing engine
- [ ] Integrate the UI layer / Front-end interface
- [ ] Connect the local database backup module

### Phase 3: Polish & Deployment
- [ ] Write unit tests and optimize performance
- [ ] Create detailed build documentation

# Blitz 3D Trading Platform Engine

![C++ Version](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Vulkan Version](https://img.shields.io/badge/Vulkan-1.3-red.svg)
![Build System](https://img.shields.io/badge/CMake-3.25%2B-gree.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

Blitz is a high-performance, real-time 3D data visualization and trading platform engine built from the ground up using **Modern C++20** and the **Vulkan 1.3** graphics API. Moving away from legacy fixed-function rendering pipelines, this engine leverages state-of-the-art Vulkan features like **Dynamic Rendering** and highly optimized **Multi-Frame In-Flight Synchronization** architectures to achieve ultra-low latency frame pacing and crash-resilient hardware execution.

---

## 🚀 Key Architectural Highlights

* **Vulkan 1.3 Dynamic Rendering Framework:** Bypasses legacy, verbose global Render Pass / Framebuffer infrastructure structures entirely by leveraging immediate, flexible dynamic layout barriers (`vkCmdBeginRendering`) to dramatically reduce driver profiling overhead.
* **Asynchronous Multi-Frame Parallelism (Double Buffering):** Implements a strict dual-lane frame pipeline matching discrete CPU gating fences (`VkFence`) with separate hardware canvas layout locks (`VkSemaphore`). This allows the CPU to record commands smoothly ahead while the GPU concurrently renders active frames.
* **Automated Shader Cross-Compilation Pipeline:** Fully integrated into CMake. Whenever changes are made to human-readable `.vert` or `.frag` GLSL shaders, the build environment invokes the Vulkan SDK `glslc` compiler behind the scenes, cross-compiling code automatically into binary 32-bit SPIR-V (`.spv`) structures.
* **Robust Driver Lifecycle Validation:** Integrated with the official Khronos validation suite and asynchronous error callback layers (`VK_EXT_debug_utils`) alongside explicit reverse-dependency destruction to enforce a 100% leak-free, clean-shutdown execution lifecycle.

---

## 🛠️ Tech Stack & Dependencies

* **Language Standard:** C++20 (Enforced compiler-side with strict `/W4 /WX` warning rules)
* **Graphics API:** Vulkan SDK (v1.3.x)
* **Windowing & Input:** GLFW (Fetched automatically via CMake)
* **Function Pointer Loader:** Volk (Meta-loader initializing global, instance, and device-level layers)
* **Build System:** Cross-platform CMake (v3.25+)

---

## 📁 Repository Structure

```text
Root/
├── Project/
│   ├── include/          # Engine header files (.h)
│   │   └── core/
│   │       └── Application.h
│   ├── source/           # Modularized rendering implementations (.cpp)
│   │   ├── ApplicationCommand.cpp      # Pipeline recording & Frame loop
│   │   ├── ApplicationDevice.cpp       # GPU enumeration & Logical devices
│   │   ├── ApplicationInIt.cpp         # Window context & Lifecycle cleanup
│   │   └── ApplicationSwapChain.cpp    # Swapchain & Image view allocation
│   └── shaders/          # Human-readable GLSL source code
│       ├── vert.vert     # Vertex stage asset
│       └── frag.frag     # Fragment/Pixel stage asset
├── Dependencies/         # Automatic dependency fetching submodule maps
├── CMakeLists.txt        # Master automation build script script
└── README.md             # This documentation profile
