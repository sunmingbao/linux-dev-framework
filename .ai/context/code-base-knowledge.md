# Linux Dev Framework Knowledge Base

## Project Overview
`linux-dev-framework` is a C-based framework designed to accelerate the development of Linux applications and libraries. It provides a structured directory layout, a robust build system using GNU Make, and a set of core utility libraries commonly needed in Linux systems programming.

## Project Structure
- **`libs/`**: Contains shared libraries. Each subdirectory represents a library (e.g., `app_utils`).
- **`apps/`**: Contains complex multi-file applications. Each subdirectory represents an application (e.g., `hello`).
- **`simple_apps/`**: Contains simple single-file utility or test applications. Each `.c` file is compiled into a standalone executable.
- **`misc/`**: Miscellaneous scripts, kernel module source code, and configuration files.
- **`target/`**: The output directory for compiled objects and executables (not part of the source tree, generated at build time).

## Build System
The project uses GNU Make with a root `makefile` and several helper scripts:
- **`make_libs`**: Manages the compilation of libraries in `libs/`.
- **`make_apps`**: Manages the compilation of applications in `apps/`.
- **`make_simple_apps`**: Manages the compilation of single-file apps in `simple_apps/`.

### Key Make Features:
- **Cross-Compilation**: Supports cross-compilation via the `CROSS_COMPILE` variable (e.g., `make CROSS_COMPILE=arm-linux-`).
- **Automatic Inclusion**: All headers in `libs/*/api/` are automatically added to the include path for all applications.
- **Automatic Linking**: All libraries in `libs/` are automatically linked into applications.

## Core Utilities (`app_utils` Library)
The framework provides several core utilities in `libs/app_utils/api/`:
- **Logging (`log.h`)**: Simple file-based logging with support for system and error logs.
- **Debugging (`debug.h`)**: Provides macros for debug printing.
- **Networking (`net.h`, `socket.h`)**: Utilities for network communications and socket management.
- **Data Structures**:
  - **`list.h`**: A Linux kernel-style doubly linked list implementation.
  - **`pc_queue.h`**: A Producer-Consumer queue implementation.
- **Terminal Control (`tty_cfg.h`)**: Utilities for terminal configuration (e.g., raw mode).
- **System Utilities**:
  - `sys_utils.h`: General system utilities.
  - `io_utils.h`: I/O operation wrappers.
  - `string_utils.h`: String manipulation helpers.
  - `timer_utils.h`: Timer-related functions.
- **Security/Hashing**:
  - `md5.h`: MD5 hashing implementation.

## Usage Conventions
- Applications should include library headers directly (e.g., `#include "log.h"`) without providing the full path, as the build system handles inclusion.
- Library implementations are found in `libs/*/src/`, and public APIs are in `libs/*/api/`.
- New libraries should follow the structure of existing ones in `libs/`.
- New multi-file applications should be placed in their own subdirectory under `apps/`.
- New single-file utilities can be placed directly in `simple_apps/`.

## Key Files
- `readme.txt`: Project introduction (GBK encoded).
- `Build.txt`: Quick build instructions.
- `makefile`: Root makefile for the entire project.
- `AGENTS.md`: Agent-related documentation for the framework.
