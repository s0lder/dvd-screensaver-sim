# dvd-screensaver-sim
 
A real-time recreation of the classic bouncing DVD logo screensaver, written in C++ with modern OpenGL (3.3 core profile).
 
The logo drifts across the window, bounces off the edges and changes color on every hit. If it ever lands exactly in a corner, it flashes a rainbow gradient instead.
 
## Features
 
- **Custom shaders:** GLSL vertex and fragment shaders handle the movement offset, texturing and color tinting.
- **Textured quad:** the logo is a transparent PNG loaded with [stb_image](https://github.com/nothings/stb) and drawn on a quad (VAO, VBO and EBO).
- **Random color on every bounce:** a new tint is picked each time the logo hits a wall.
- **Corner detection:** a hit on two walls at once triggers the rainbow vertex-color gradient.
- **Separate logic thread:** physics runs in its own `std::thread` and shares state with the render loop through `std::atomic`, so rendering never waits on game logic.
- **Frame-rate independent movement:** velocity is multiplied by the measured delta time, with a short collision debounce to avoid double bounces.
## How it works
 
```
 logic thread (~60 Hz)                    render thread (main)
 ---------------------                    ---------------------
 measure delta time                       clear screen
 move logo (position += velocity * dt)    set position uniform from atomics
 check walls, flip velocity               react to collision / corner flags
 raise collision / corner flags  ----->   draw textured quad
                                          swap buffers, poll events
```
 
| File | Purpose |
| --- | --- |
| `Main.cpp` | Window and OpenGL setup, render loop, logic thread |
| `Shader.h` / `Shader.cpp` | Small wrapper that loads, compiles and links GLSL shaders |
| `vertex.glsl` | Offsets the quad by the current position |
| `fragment.glsl` | Samples the logo texture and applies the tint |
| `stb_image.h` / `stb_image.cpp` | Image loading |
| `dvd_logo_no_bg.png` | Logo texture (transparent background) |
 
## Building
 
The project is a Visual Studio solution (`dvd-screensaver-sim.sln`) targeting the v143 toolset with C++20.
 
**Requirements**
 
- Visual Studio 2022 with the "Desktop development with C++" workload
- [GLFW 3](https://www.glfw.org/) (the project links `glfw3.lib` and `opengl32.lib`)
- A [GLAD](https://glad.dav1d.de/) loader generated for **OpenGL 3.3, Core profile**
**Steps**
 
1. Clone the repository:
```
   git clone https://github.com/s0lder/dvd-screensaver-sim.git
```
2. Put the generated `glad.c` in the project folder (`dvd-screensaver-sim/dvd-screensaver-sim/`). It is git-ignored, so you need to add it yourself. Add the `glad/` and `KHR/` headers to your include path.
3. Make GLFW's headers and `glfw3.lib` visible to Visual Studio (for example through vcpkg or by adding the include and library directories to the project).
4. Open `dvd-screensaver-sim.sln`, select **x64**, and press **Ctrl+F5**.
The executable loads `vertex.glsl`, `fragment.glsl` and `dvd_logo_no_bg.png` from the working directory, so run it from the project folder (Visual Studio does this by default).
 
## Usage
 
Run it and watch. The window is 800x600 and not resizable. Close the window to exit.
 
## License
Released under the [MIT License](LICENSE).
