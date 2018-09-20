# 10ft
Standalone/VST audio editor.

### Features
- Load/record audio
- Play/loop/pause/stop audio
- Mute/fade in/fade out/normalize selected region
- Can be loaded into a DAW as a VST plugin
  
### Build
- Using [JUCE](https://juce.com/)
  1. Install JUCE
  2. Open 10ft.jucer & build
- Using [CMake](https://cmake.org/)
  1. Install CMake
  2. Build using the CMake script:
        ```
        cd <10FT_HOME>
        mkdir Builds && cd Builds/
        cmake ..
        cmake --build .
        ```
