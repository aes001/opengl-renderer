# OpenGL Renderer

This project is an OpenGL based rendering engine that demonstrates basic rendering techniques using modern OpenGL. It includes 3D object rendering, texture mapping, lighting, camera controls, split screen view, a simple GUI system, animation, and particle effects.

## Building

This project uses Premake to generate build files. Premake is included in the top level directory of the project.
Note that **this project does not support out of source builds**, so please run Premake from the root directory of the project.

### Requirements
----------------
- C++23 compatible compiler (We've tested this project with MSVC and GCC)

#### Building on Windows

Visual Studio 2022 or newer is required to build this project on Windows.
``` PowerShell
git clone https://github.com/aes001/opengl-renderer.git
.\premake5 vs2022
.\COMP3811-glcode.sln
```
Then build the solution in Visual Studio.

#### Building on Linux
``` BASH
git clone https://github.com/aes001/opengl-renderer.git
../premake5 gmake
make
```

## Running

After building the project, you can run the executable located in the bin directory.

## Controls

### Movement and Camera Controls
|                 |                             |
|-----------------|-----------------------------|
|`RMB`            | Enable/Disable mouse look   |
|`Mouse Movement` | Look around                 |
|`W` `A` `S` `D`  | Move the camera             |
|`Q` `E`          | Move the camera up and down |
|`Shift`          | Increase movement speed     |

### Animation Controls

|    |                    |
|----|--------------------|
|`F` |Play/pause animation|
|`R` |Reset animation     |

### Viewport Controls

|             |                                                                 |
|-------------|-----------------------------------------------------------------|
|`C`          | Change camera mode                                              |
|`V`          | Toggle split screen view                                        |
|`Shift` + `C`| Change camera mode for the second viewport in split screen mode |

### Other Controls
|       |                                   |
|-------|-----------------------------------|
|`1`-`4`|Turn on/off different light sources|
|`Esc`  |Exit the application               |



