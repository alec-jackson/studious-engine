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

### libraries
- SDL2
- SDL2_mixer
- SDL2_image
- GLEW
- glm
- freetype2
- googletest

### How to use this
This started as an expansion of a pong engine we wrote so we didn't properly separate engine features and gameplay features. This is something we're working on planning on fixing while we're implementing the last of the essential features. (Text, Collision, Animation) We're planning to get this to a place where the engine can be called like a library, but right now some of the app level features are running inside our source files.

If you'd like to use it right now you'll probably need to do a fair amount of work on get things working as needed. Although we hope it'll provide a good starting reference for working with OpenGL.


### Other Projects From Us
[OpenGL Pong](https://github.com/alec-jackson/OpenGLPong) a OpenGL version of the original Atari pong.
