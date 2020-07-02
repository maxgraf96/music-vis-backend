# music-vis-backend

*Note: Only macOS is supported at the moment.*

## Installation - Use
1. Grab the current [release](https://github.com/maxgraf96/music-vis-backend/releases)
2. Install dependencies
- If you are using [Homebrew](https://brew.sh/), open a terminal and run `brew install liblo` to install [liblo](http://liblo.sourceforge.net/). As of now, liblo is the only dependency that can be installed via Homebrew. Navigate to the release folder and run `./install_libmapper.sh` to install [libmapper](https://libmapper.github.io/) as well as `./install_fftw.sh` to install [FFTW](http://www.fftw.org/).
- If you are not using Homebrew, navigate to the release folder in your terminal and run `./install_dependencies.sh`. 
If you do not have the Apple Developer Tools installed on your machine yet, you will be prompted to download and install them. They are necessary to compile C/C++ code on macOS. Following that the script will install FFTW, liblo and libmapper to your machine (specifically, to `/usr/local/include` and `/usr/local/lib`).
3. Execute `music-vis-backend-package.pkg` and follow the installation process. This will install a standalone version of the software (in your Applications folder) as well as an audio plugin in the VST3 format
4. Open your favourite DAW (Ableton Live) and add the `music-vis-backend` plugin to a track
5. Open the `WebMapper` application in the release folder. (*Optional: Drag the `WebMapper` application to your `Applications` folder for easy access*)
6. ??? (Frontend connection to 3D visualisation coming soon)
7. Profit :)

## Installation - Development
todo...
