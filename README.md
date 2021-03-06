# GBEmu

A simple emulator for the original Nintendo Gameboy, built as a pet project just for fun and learning.


## Screenshots



|              Title Screen               |                 Gameplay               |
|-----------------------------------------|----------------------------------------|
|   ![Zelda1](screenshots/Zelda1.png)     |    ![Zelda2](screenshots/Zelda2.png)   |
| ![Pokemon1](screenshots/Pokemon1.png)  |  ![Pokemon2](screenshots/Pokemon2.png) |


## Setup and building

Before running the project, install SDL2 from https://www.libsdl.org/

SDL2 was used for all graphics purposes

As a warning, this was only tested on my local Macbook with macOS Mojave.

### Cloning the rep
After cloning as normal, run
```
git submodule update --init --recursive
git submodule update --recursive --remote
```
to fetch the logging library used in this project

### Running the project 
Add any roms you want to run to the `roms` directory, and then change the string in `main.cc` at line 33 to be the file name of the rom.

In the future, this will be fixed to be a command line argument

Then, build the project using CMake. I run the following commands to build the project to a `build` directory
```
mkdir build
cd build
cmake ..
make
```

#### Controls
| Computer Key | Gameboy Button |
|--------------|----------------|
|       S      |        B       |
|       A      |        A       |
|   ARROW UP   |    JOYPAD UP   |
|  ARROW DOWN  |   JOYPAD DOWN  |
|  ARROW LEFT  |   JOYPAD LEFT  |
| ARROW RIGHT  |  JOYPAD RIGHT  |
|    SPACE     |      START     |
|    ENTER     |     SELECT     |

Finally, you can run the project using `./GBEmu <path_to_rom>`
## Acknowledgements
A great tool for Gameboy development is https://github.com/gbdev/awesome-gbdev, which a curated list of useful resources.

Of those resources, huge ups to https://gbdev.io/pandocs/, most of the technical information about the Gameboy I got from here.
