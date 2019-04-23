# OpenHMDDemo

OpenHMDDemo is a application written in Ogre (currently using stock 1.9) that shows a basic implementation of OpenHMD into a working functional framework.

![alt tag](http://s18.postimg.org/mffh0c3pl/Open_HMD_Demo_App.png)

Currently, just the basics are working, but a more pretty demo scene and some additional features will be added later on.

**Working:**
- Functional OpenGL 2.1
- DotSceneLoader based on tinyxml
- OpenHMD integrated with Oculus DK1 and DK2 in mind (1280x800 and 1920x1080 with 2x oversampling shaders)
- Wireframe rendering toggle (r)
- Commandline level loader with -l argument (all standard ogre .scene files should work)
- CMake!

**To-Do:**
- Basic collision system
- Simple shaders (Primarily Normalmapping)
- Finish demo scene! (something fancy in the works)
- Modern GL version using Ogre 1.10 with newer GL 3.3+ renderer

## Compiling

The code uses default Ogre 1.9, tested with a boost compiled version from the Ubuntu 14.04 repository (non boost versions should work as well).

### Debian/Ubuntu based distro's

There are some basic dependencies to be able to compile this project.
```
sudo apt-get install libogre-1.9-dev libois-dev libtinyxml-dev
```

OpenHMD is required as well and can either be compiled from trunk (recommended) or installed from some repositories
like Debian Sid or Ubuntu Zesty.

CMake is used for building, if libraries are missing it will prompt which ones are missing.
Basic compilation should be:
```
cmake .
make
```
