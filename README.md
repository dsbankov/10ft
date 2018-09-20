# 10ft
Standalone/VST audio editor.

### Features
- Load/record audio
- Playback - play/loop/pause/stop
- Edit - mute/fade in/fade out/normalize selected region
- Can be loaded into a DAW as a VST plugin
  
### Build

- using JUCE
1. Install JUCE
2. Open 10ft.jucer & build

- using CMake
1. Install CMake
2. Execute the build script:
$ cd <10FT_HOME>
$ mkdir Builds && cd Builds/
$ cmake ..
$ cmake --build .
