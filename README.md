# Bird Flight Prototype

A responsive bird-flight gameplay prototype built in Unreal Engine 5.7 using C++ and Blueprints, focused on flight feel, state-driven movement, visual banking, gliding, and camera feedback.

## Overview

Bird Flight Prototype is a small gameplay-focused portfolio project designed to demonstrate responsive character movement and gameplay programming.

The goal was not to build a realistic bird simulator. The goal was to create a compact playable experience where another person can immediately take off, steer, glide, bank through the environment, and feel the responsiveness of the controller.

Most gameplay movement and state logic is implemented in C++, while Unreal Blueprints are used for animation, input assets, UI, and project setup.

## The Problem

Flight controllers can easily feel either too mechanical or too simulation-heavy.

For this prototype, I wanted the bird to:

- Respond immediately to player input
- Retain smooth visual motion
- Transition clearly between powered flight and gliding
- Preserve momentum during state changes
- Communicate speed through camera feedback
- Remain simple enough to understand and maintain

## Controls

- **Hold W** — Powered Flight
- **Mouse** — Steer
- **Release W** — Glide

The playable showcase also contains three glowing rings that give the player a simple direction to fly through.

## Core Features

- Grounded, Powered Flight, and Gliding gameplay states
- Smooth acceleration and deceleration
- Mouse-driven pitch and yaw steering
- Smoothed visual banking
- Momentum-preserving glide transitions
- Controlled glide descent
- Smooth procedural takeoff assist
- Ground-proximity landing detection
- Speed-driven dynamic camera FOV
- Third-person camera lag
- Animation Blueprint integration
- Minimal gameplay HUD

## Gameplay Flow

```text
Grounded
   |
   | Hold W
   v
Powered Flight
   |
   | Release W
   v
Gliding
   |
   | Hold W
   v
Powered Flight

Gliding + Ground Detection
   |
   v
Grounded
```

## Architecture

### `ABirdPawn`

The main C++ Pawn owns the gameplay flight controller.

Responsibilities include:

- Flight state management
- Forward acceleration
- Pitch/yaw steering
- Visual bank calculations
- Glide movement
- Takeoff assistance
- Landing detection
- Dynamic camera FOV
- Enhanced Input bindings

### `EFlightState`

The controller intentionally uses only three gameplay states:

```text
Grounded
PoweredFlight
Gliding
```

Takeoff and landing animations are handled visually rather than adding unnecessary gameplay states.

### Animation Blueprint

The Animation Blueprint reads the current C++ flight state and uses it to drive the bird animation state machine.

The gameplay system remains authoritative over movement while animation responds to the current gameplay state.

## Key Technical Decisions

### Small State Model

Instead of creating separate gameplay states for every animation phase, the controller keeps only the states that affect gameplay behavior.

This keeps transitions easier to reason about and avoids unnecessary state complexity.

### Gameplay Response vs Visual Response

Actual steering responds directly to player input.

Visual banking uses separate smoothing so the controller can remain responsive while the bird still feels organic.

### Glide Momentum

Entering Glide does not automatically accelerate the bird.

If the bird enters Glide above the configured glide speed, it decelerates toward it. If it enters below that speed, the existing slower momentum is preserved.

### Dynamic Camera FOV

Camera FOV is driven by actual forward speed rather than simply switching between fixed values for each flight state.

This allows the camera intensity to naturally increase as the bird accelerates.

## Project Structure

```text
BirdFlightProto/
├── Config/
├── Content/
│   ├── Blueprint/
│   ├── Core/
│   └── Maps/
├── Source/
│   └── BirdFlightProto/
│       ├── Public/
│       │   └── BirdPawn.h
│       └── Private/
│           └── BirdPawn.cpp
└── BirdFlightProto.uproject
```

## Running the Project

This repository contains the original gameplay code and project-specific assets that can be redistributed.

The complete playable showcase uses third-party assets that are intentionally not included in this repository.

### Requirements

- Unreal Engine 5.7
- Visual Studio with Unreal Engine C++ development support

### Setup

1. Clone the repository.
2. Open `BirdFlightProto.uproject`.
3. Generate project files if Unreal requests them.
4. Compile the C++ project.
5. Add the required external visual assets if reproducing the full showcase environment.

## Third-Party Content

Third-party assets used for the playable presentation are intentionally excluded from this public repository.

These include:

- Bird mesh and animation assets
- Environment assets
- External font assets

They are used only for presentation and are not part of my original gameplay implementation.

The C++ flight controller, state logic, camera behavior, project-specific Blueprints, input setup, and gameplay implementation are my work.

## Current Scope

This prototype currently includes:

- Takeoff
- Powered flight
- Steering
- Smooth banking
- Gliding
- Landing
- Camera feedback
- Animation integration
- Minimal HUD
- Simple visual flight guidance

## Out of Scope

The project intentionally does not include:

- Realistic aerodynamic simulation
- Lift/drag modeling
- Stalls
- Wind simulation
- Stamina systems
- Combat
- Procedural bird IK
- Large mission systems

The focus is responsive gameplay feel rather than simulation depth.

## Built With

- Unreal Engine 5.7
- C++
- Unreal Engine Blueprints
- Enhanced Input
- Animation Blueprints

## Status

**Portfolio Prototype — Complete**
