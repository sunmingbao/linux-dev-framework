# Global Rules for linux-dev-framework

## Project Overview
This project is a Linux development framework in C, providing common utilities and a structured build environment for Linux applications and libraries.

## Directory Structure
- `libs/<lib_name>/`: Library directory.
    - `api/`: Public header files (exported to other components).
    - `src/`: Source code files.
- `apps/<app_name>/`: Application directory. All `.c` files in this directory will be compiled into a single executable named after the directory.
- `simple_apps/`: Each `.c` file in this directory is a standalone application, compiled into an executable with the same name.
- `misc/`: Miscellaneous scripts, kernel modules, and experimental code.
- `target/`: Build artifacts (binaries, objects, libraries). Do not commit this directory.
- `.ai/`: AI coding agent support directory.
    - `skills/`: Specialized skill definitions or documentation.
    - `context/`: Project-specific context files and background information.
    - `prompts/`: Reusable prompt templates or instructions.
    - `archive/`: Archived session logs or temporary research data.

## Coding Standards
- **Language**: C (GNU dialect).
- **Indentation**: 4 spaces. No tabs.
- **Naming Conventions**:
    - Functions: `snake_case` (e.g., `init_log`).
    - Variables: `snake_case` (e.g., `fd_log_file`).
    - Macros: `UPPER_CASE` (e.g., `ARRAY_SIZE`).
    - Header Guards: `__HEADER_NAME_H__`.
- **Header Files**:
    - Always use include guards.
    - Public APIs must be in the `api/` directory of a library.
- **Error Handling**:
    - Prefer returning `int` (0 for success, non-zero for error).
    - Use `SysLog`, `ErrSysLog`, and `ErrSysLogQuit` (defined in `libs/app_utils/api/log.h`) for logging and error reporting.
- **Memory Management**:
    - Always check return values of `malloc`/`calloc`.
    - Ensure allocated memory is freed when no longer needed, unless the application is exiting.

## Build System
- Use the root `makefile` to build the entire project.
- The build system automatically discovers libraries in `libs/`, apps in `apps/`, and simple apps in `simple_apps/`.
- Support for cross-compilation via `CROSS_COMPILE` (e.g., `make CROSS_COMPILE=arm-linux-`).
- Compile types: `release` (default, `-O2`) and `debug` (`-g -D_DEBUG`).

## Workflow for Adding New Components
1. **Adding a Library**:
    - Create directory `libs/<new_lib_name>`.
    - Create `libs/<new_lib_name>/api` for public headers and `libs/<new_lib_name>/src` for source files.
2. **Adding an Application**:
    - Create directory `apps/<new_app_name>`.
    - Place all source files in this directory.
3. **Adding a Simple App**:
    - Add a single `.c` file to `simple_apps/`.

## Documentation
- API documentation should be provided as comments in the header files within `api/` directories.
- High-level project information is maintained in `readme.txt`.

## Maintenance of Global Rules and Context
- This `AGENTS.md` file is the source of truth for project-level conventions and rules.
- If any action (e.g., refactoring, changing the build system, or adopting new coding standards) changes the project's structure or conventions, this file **must** be updated immediately to reflect those changes.
- The `.ai/context/code-base-knowledge.md` file provides a high-level overview of the project's architecture, utilities, and build system. It **must** be updated automatically whenever any changes are made that affect the information documented in it, ensuring it remains an accurate representation of the project's state.
