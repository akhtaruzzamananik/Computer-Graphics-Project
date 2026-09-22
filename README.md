# 🚇 DIU Metro Rail Simulation

A Computer Graphics project developed using **C++ and OpenGL/GLUT**.

This project presents an animated metro rail simulation based on a **DIU campus environment**, featuring buildings, a metro station, railway tracks, signals, passengers, clouds, and an animated metro train.

---

## 📌 Project Overview

The simulation demonstrates various **Computer Graphics algorithms and concepts** through an interactive 2D graphics scene.

The main scene includes:

- DIU campus buildings
- DIU Metro Station
- Railway tracks
- Moving metro train
- Train doors with animation
- Passengers boarding the train
- Railway signal system
- Moving clouds
- Sun
- Pseudo-3D train effects

The project is implemented as a **single C++ source file** using OpenGL/GLUT.

---

## ✨ Features

### 🏢 Campus Buildings

The scene contains five buildings:

- RASG-1
- YKSG-1
- AB4
- RASG-2
- AB1

Each building contains windows and a blue signboard displaying its name.

### 🚉 DIU Metro Station

A dedicated station platform is included with:

- Station platform
- Safety line
- Station wall
- `DIU METRO STATION` signboard

### 🚇 Metro Train

The train contains:

- 3 coaches
- Engine/cab
- Windows
- Wheels
- Doors
- Pseudo-3D effects

The train moves toward the station, stops, opens its doors, allows passengers to board, closes the doors, and then leaves the station.

### 🚦 Signal System

The simulation includes a railway signal with:

- Red signal
- Green signal

The signal changes according to the train's state.

### 🧍 Passenger Animation

Passengers wait at the station and move toward the train door during the boarding phase.

### ☁️ Animated Environment

The scene also includes moving clouds and a static sun to create a daytime environment.

---

## 🧮 Computer Graphics Algorithms Used

### 1. DDA Line Drawing Algorithm

Used for drawing selected building outlines and line segments.

### 2. Bresenham's Line Drawing Algorithm

Used for:

- Railway tracks
- Platform safety line
- Signal elements
- Building outlines
- Train edges

### 3. Midpoint Circle Algorithm

Used for drawing circular objects such as:

- Signal lights
- Passenger heads
- Cloud outlines

### 4. OpenGL Primitive Rendering

The project uses OpenGL primitives such as:

- `GL_POINTS`
- `GL_QUADS`
- `GL_TRIANGLE_FAN`
- `GL_LINE_LOOP`

---

## 🎬 Train State Machine

The train animation is controlled using a state machine.

The states include:

```text
MOVING_TO_STATION
        ↓
ARRIVING
        ↓
STOPPED_SIGNAL_RED
        ↓
DOORS_OPENING
        ↓
PASSENGERS_BOARDING
        ↓
DOORS_CLOSING
        ↓
SIGNAL_GREEN_WAIT
        ↓
MOVING_AWAY
        ↓
MOVING_TO_STATION
