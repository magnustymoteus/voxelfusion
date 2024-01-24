# VoxelFusion
## Dependencies
### JSON
Get [the JSON library](https://github.com/nlohmann/json) and place `json.hpp` at `src/lib/json.hpp` 
### GLM
Get [GLM v0.9.9.8](https://github.com/g-truc/glm/releases/tag/0.9.9.8) and copy the `glm` folder such that `src/lib/glm/glm.hpp` exists
### ImGui
Get [ImGui](https://github.com/ocornut/imgui) and copy all the .cpp and .h files from the root folder into `src/lib/imgui`, and copy everything from the `backends` folder with `opengl3` or `glfw` in its name to `src/lib/imgui` as well.
### GLFW
#### Linux
Install libglfw3/libglfw3-dev through your package manager
#### Windows
- download the [GLFW source](https://www.glfw.org/download.html)
- Compile and install GLFW with CMake, example for Ninja/MinGW bundled with CLion:
  - add the path to your compiler to the PATH environment variable, e.g. `C:\Program Files\JetBrains\CLion 2022.3\bin\mingw\bin`
  - now, in the glfw source folder, run
    - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -S . -B build`
    - `cmake --build build`
    - `cmake --install build --prefix install`
- Copy the contents of the installed folder to `src/lib/GLFW`

Everything should now be set up correctly.
