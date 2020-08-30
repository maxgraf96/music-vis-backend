# Interstell.AR - Backend
This is the repository for the backend of the Interstell.AR software system. Although the source code
is for the backend, the release channel of this repository contains all components of the system at the current point.

*Note: Only macOS Mojave (10.14) or higher is supported at the moment.*

## Installation - Use
1. Grab the current [release](https://github.com/maxgraf96/music-vis-backend/releases)
2. Install dependencies
    - Open a terminal
    - Navigate to the unzipped release folder
    - Execute `./install_dependencies.sh`
        - Note: If you do not have the Apple Developer Tools installed on your machine yet, you will be prompted to download and install them. 
        They are necessary to compile C/C++ code on macOS. Following that, the script will install FFTW, liblo and libmapper to your machine (specifically, to `/usr/local/include` and `/usr/local/lib`).
        - You may have to re-run the `./install_dependencies.sh` command if you did not have the Apple Developer Tools installed already.
        - If you already have some of the required dependencies installed, 
        you can instead navigate into the `dependencies` folder and use the dedicated installer scripts `install_fftw.sh`, `install_liblo.sh` and `install_libmapper.sh`.
3. Open `interstellar_installer.pkg` and follow the installation process. 
This will install:
    - A standalone version of the backend
    - Audio plugins in the VST3 and AU formats
    - The "interstellar" application (the frontend of the system producing the visualisations)
    - The "Webmapper" interface application
4. Open your favourite digital audio workstation and add the `music-vis-backend` plugin to a track
5. Open the "interstellar" application in your Applications folder.
6. Open the "Webmapper" application in your Applications folder.
7. In the Webmapper interface you should now see one device named "music-vis-backend-libmapper.1". 
Additionally, you should see devices for all visual objects in the frontend. (See screenshot below)

![](https://i.imgur.com/w6lkJiE.png)

## Uninstallation
If you decide to remove the software from your system:
1. Open a terminal
2. Navigate to the unzipped release folder
3. Navigate to the folder `uninstall`
4. Uninstallation
    - Run `./uninstall_everything.sh` to remove both the dependencies and the software (the plugins, the frontend application and the Webmapper application)
    - Run `./uninstall_dependencies.sh` if you want to remove the dependencies but keep the software.
    - Run `./uninstall_apps.sh` if you want to remove the software but keep the dependencies.

## Installation - Development
1. Grab the current [release](https://github.com/maxgraf96/music-vis-backend/releases)
2. Install dependencies
    - Open a terminal
    - Navigate to the unzipped release folder
    - Execute `./install_dependencies.sh`
        - Note: If you do not have the Apple Developer Tools installed on your machine yet, you will be prompted to download and install them. 
        They are necessary to compile C/C++ code on macOS. Following that, the script will install FFTW, liblo and libmapper to your machine (specifically, to `/usr/local/include` and `/usr/local/lib`).
        - You may have to re-run the `./install_dependencies.sh` command if you did not have the Apple Developer Tools installed already.
        - If you already have some of the required dependencies installed, 
        you can instead navigate into the `dependencies` folder and use the dedicated installer scripts `install_fftw.sh`, `install_liblo.sh` and `install_libmapper.sh`.
3. Download and install [JUCE](https://juce.com/)
4. Download the source code
5. Make sure that there are no whitespaces in the path leading to your source folder
6. Open the `music-vis-backend` folder in the IDE of your choice
7. Open the `CMakeLists.txt` file
8. Go to line 23 and change `"~/IJUCE"` in `list(APPEND CMAKE_PREFIX_PATH "~/IJUCE")` to your JUCE installation folder path
9. Run one of the CMake configurations
    - `music-vis-backend_VST3` will build the VST3 plugin
    - `music-vis-backend_AU` will build the AU plugin
    - `music-vis-backend_Standalone` will build a standalone executable version (*.app file) of the software
10. Results of the build process will be in the `cmake-build-debug/music-vis-backend_artefacts` or 
`cmake-build-release/music-vis-backend_artefacts` folders, depending on your configuration
11. Code away :)