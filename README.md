# OpenHMDDemo

OpenHMDDemo is a application written in Ogre (currently using stock 1.9) that shows a basic implementation of OpenHMD into a working functional framework.

![alt tag](http://s18.postimg.org/mffh0c3pl/Open_HMD_Demo_App.png)

Currently, just the basics are working, but a more pretty demo scene and some additional features will be added later on.

**Working:**
- Functional OpenGL 2.1
- DotSceneLoader based on timyxml
- OpenHMD integrated with DK1 in mind (1280x800 resolution Shaders)
- Wireframe rendering toggle (r)
- Commandline level loader with -l argument (all standard ogre .scene files should work)

**To-Do:**
- Basic collision system
- Simple shaders (Primarily Normalmapping)
- Makefile (or cmake for easier platform compilation)
- Finish demo scene! (something fancy in the works)

##Compiling

The code uses default Ogre 1.9, tested with a boost compiled version from the Ubuntu 14.04 repository (non boost versions should work as well).

###Debian/Ubuntu based distro's

There are some basic dependencies to be able to compile this project.
>sudo apt-get install libogre-1.9-dev libois-dev libtinyxml2-dev

Compile using the current horrible compile.sh (i know, i should write a makefile)
