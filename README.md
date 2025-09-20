# OpenGL Game Engine
## Christian Galvez and Alec Jackson

### August 2023 Build

![Studious GIF Small](https://github.com/alec-jackson/studious-engine/assets/42315696/d287bf0a-e415-407b-a88e-39902169ddfb)

### Current Features
- Interaction with SDL to draw window
- Loading Shaders
- Object Loader
- Global Lighting
- Destroy Objects
- Multiple Objects
- Keyboard Bindings
- GamePad Bindings
- CMake Build System
- Object modification (Rotation, Movement, Scaling)
- Collision

### Planned Features
- --- Improved Usability ---
- Physics
- Animations
- Expanded Control System (Mouse and better GamePad support)
- Better Lighting
- Text

-- A note on improved usability, we're in the process of removing pulling setup out of our core files and into a reference launcher file. This is more desirable so our library functions can be called to assist the program not be the program. Additionally, as the library grows the library will not need to be compiled with the program saving significant compile time.


### Design Principals
- Cross Platform Support
- Use of libraries that maximize platform support
- Use of low level compiled languages (C & C++) for performance
- Use of class objects to manage elements in the scene

### Building
We're using cmake for cross platform support. This allows us to work across multiple platforms without to many build concerns. We're planning to create a script that will fetch the required libraries, however as of now that will need to be done manually. Check the libraries section to figure out what libraries we're using. For those new to cmake we've made a super simple script for setting things up just right for this project.

-- A note on windows support. With cmake there should be windows support, but this is untested. This may require messing around with the threading libraries to get things working right with windows.

To compile the studious engine in debug mode, pass the -d flag as a command line argument when running the setupBuild.sh shell script.

To build and run the 3D demo built into studious, run the following command:

`./setupBuild.sh -r`

To build other included demo targets, specify the target to build and run with the `-target` flag:

`./setupBuild.sh -r -target studious-2dExampleScene`

### libraries
- SDL2
- SDL2_mixer
- SDL2_image
- glm
- freetype2
- googletest

### How to use this
This started as an expansion of a pong engine we wrote so we didn't properly separate engine features and gameplay features. This is something we're working on planning on fixing while we're implementing the last of the essential features. (Text, Collision, Animation) We're planning to get this to a place where the engine can be called like a library, but right now some of the app level features are running inside our source files.

If you'd like to use it right now you'll probably need to do a fair amount of work on get things working as needed. Although we hope it'll provide a good starting reference for working with OpenGL.

## Installing The Studious Library

The studious library can be installed by simply passing the `-i` argument to the buildScript. This will run cmake --install on your build directory, and add the compiled library and headers to your sysroot. The complete command to execute from the project's directory is:

`./setupBuild.sh -i`

On MacOS the above command should work fine, but on Linux hosts you may need to run the build script with sudo privileges.

## Studious Template Project

After installing the studious library on your system, you can clone the template project at https://github.com/Weetsy/studious-template and run its setupBuild script to run an empty base project. You can compile and run this new studious project the same way you build and run the engine example:

`./setupBuild.sh -r`

This template will need to be periodically updated when breaking changes are made to certain base components in studious. If this template project no longer compiles, file an issue in the studious repository and we will fix it.

### Other Projects From Us
[OpenGL Pong](https://github.com/alec-jackson/OpenGLPong) a OpenGL version of the original Atari pong.
