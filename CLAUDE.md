# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Install uFBT: `python3 -m pip install --upgrade ufbt`
- Build application: `ufbt`
- Launch on Flipper: `ufbt launch`
- Generate VS Code configuration: `ufbt vscode_dist`
- Lint code: `ufbt lint`
- Build for release: `ufbt -c release`
- Debug application: `ufbt debug`
- Update SDK version: `ufbt update --channel=[dev|rc|release]`

## VS Code Integration
- Generate VS Code config: `ufbt vscode_dist`
- Build: Ctrl+Shift+B (or Cmd+Shift+B on macOS)
- Tasks available in Terminal > Run Task menu:
  - Launch App on Flipper
  - Build
  - Clean
  - Flash FW (SWD)
  - Flash FW (USB)
  - Open Flipper CLI session
- Debugging: Use Run & Debug panel with several configurations:
  - Attach FW (ST-Link)
  - Attach FW (DAP)
  - Attach FW (blackmagic)
  - Attach FW (JLink)
- Recommended extensions installed automatically

## Development Structure
- Use `application.fam` for app manifest configuration
- Place icons and images in `assets/` directory
- Keep private libraries in `lib/` directory
- Entry point must match application.fam's entry_point field

## SDK Structure
- SDK is installed in `~/.ufbt/` directory
- Headers are in `~/.ufbt/current/sdk_headers/f7_sdk/`
- Main API groups:
  - `furi/`: Core OS primitives and helpers
  - `furi_hal/`: Hardware abstraction layer
  - `applications/services/`: System services (GUI, dialogs, etc.)
  - `targets/f7/`: Platform-specific implementations
  - `lib/`: Utility libraries and drivers

## Code Style Guidelines
- Include blocks grouped: system headers first, then project headers
- Use `#pragma once` for header guards
- Hungarian notation for types (e.g., MhZ19App)
- Function naming: snake_case with component prefix (e.g., mh_z19_app_init)
- Structs use typedef with clear component hierarchy
- Constants in UPPER_CASE with component prefix
- Comment all functions with Doxygen-style documentation (/**...*/)
- Always check return status from API calls
- Error handling: use appropriate status codes and clean up resources
- Memory management: always free allocated resources in reverse order
- Thread safety: use mutex when accessing shared data
- Use clean resource acquisition/release pattern (init/free pairs)
- Follow Flipper Zero SDK conventions for UI, events, and threading
- Recommended stack size: 1-2 KB for most applications
- Use the FuriHal* APIs for hardware interaction
- Follow event-driven programming model with message queues