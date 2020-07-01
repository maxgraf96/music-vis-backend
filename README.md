# music-vis-backend

*Note: Only macOS is supported at the moment.*

## Installation - Use
1. Install Apple Developer Tools
2. Grab the current [release](https://github.com/maxgraf96/music-vis-backend/releases)
3. Navigate to the release folder in terminal and run `./install_dependencies.sh`. That will install [liblo](http://liblo.sourceforge.net/) as well as [libmapper](https://libmapper.github.io/) to your machine (specifically, to `/usr/local/include` and `/usr/local/lib`). If you are using [Homebrew](https://brew.sh/) you can alternatively install liblo with `brew install liblo`.
4. Execute `music-vis-backend-package.pkg` and follow the installation process. This will install a standalone version of the software as well as an audio plugin in the VST3 format
5. Open your favourite DAW (Ableton Live) and add the `music-vis-backend` plugin to a track
6. Open the `WebMapper` application in the release folder. (*Optional: Drag the `WebMapper` application to your `Applications` folder for easy access*)
7. ??? (Frontend connection to 3D visualisation coming soon)
8. Profit :)

## Installation - Development
todo...
