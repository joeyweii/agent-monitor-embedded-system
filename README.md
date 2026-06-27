# Agent Monitor

A hardware-based status monitor for autonomous AI agents using Raspberry Pi Pico (RP2040) and ST7735 LCD.

## Overview

The **Agent Monitor** provides a dedicated, glanceable display for monitoring the status of AI agents. It allows developers to keep track of agent health, tasks, and approval requests without switching windows on their primary workstation, even when away from the PC (e.g., during lunch).

## Features

- **Real-time Status Display**: View multiple agent states at a glance.
- **Physical Interaction**: 3-button navigation (PREV, NEXT, SELECT) for cycling through agents and confirming actions.
- **High-Performance Rendering**: Optimized ST7735 driver with DMA SPI support for smooth UI.
- **USB Serial Communication**: Lightweight protocol for host-to-device updates.

## Documentation

- [Design Document](docs/design_doc.md): System architecture and technical goals.
- [Project Roadmap](docs/roadmap.md): High-level development phases and status.
- [Architecture Decision Records (ADR)](docs/adr.md): Critical design choices, trade-offs. 
- [Test Report](docs/test_report.md): Test summaries covering host-compiled unit tests.
- [Hardware Configuration & Wiring](docs/breadboard_layout.md): Physical schematic, pinout assignments, and breadboard layout.
