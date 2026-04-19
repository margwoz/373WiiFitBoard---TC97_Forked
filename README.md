# EECS 373: Team 12 Fitness Board

Interface with the Wii Fitness board to control and complete a marble maze course. Includes features like a play timer, audio ques, and wireless communication between components.
Built using STM32 Micro Controller and more...

## Description

Using load cells in the corners of our board, the center of mass of the user is calculated and used to control motors to tilt the marble maze. Once the user competes the maze, a beam break sensor is triggered and the time the user takes to complete the maze is displayed. 

## Authors
- [Achal Khatri](https://github.com/akkik27)
- [Andrew Leonard](https://github.com/andrewleonard3)
- [Jackson Seifferly](https://github.com/jseifferly)
- [Margaret Wozniak](https://github.com/margwoz)

## Project Structure
```
.
├── FittnessBoard/
│   ├── src
│   │  ├── main.c           # Main fitness board control loop; reads sensors, computes CoM, wirelessly transmits to maze
|   |  └── hx711.c          # Driver for HX711 Load cell Amplifiers
│   └── inc
│      ├── main.h
│      └── hx711.h
└── Maze/
    ├── src
    │  ├── main.c            # Maze state machine control logic, manages motor control, timer, and wireless communication as well
    │  └── MAX7219Driver.c   # Driver for MAX7219 4x8x8 LED matrix display controller
    └── inc
       ├── main.h
       ├── MAX7219Driver.h
       └──  audio.h          # Stores the audio files for sound ques for game
```
