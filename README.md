# VoxelFusion
## Dependencies
### JSON
Get [the JSON library](https://github.com/nlohmann/json) and place `json.hpp` at `src/lib/json.hpp` 
### GLM
Get [GLM](https://github.com/g-truc/glm) and copy the `glm` folder such that `src/lib/glm/glm.hpp` exists
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