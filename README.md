# Kuwagata plugin for OBS Studio

## What is this?
A plugin integrating my previous bible-verse serving program, *Kuwagata*, into Open Broadcasting Software.

This plugin should work for all recent versions of OBS 30+.

## Installation
You'll need three things:
- Any number of .JSON converted bibles from an OSIS format, of which many .XML files can be found [here](https://github.com/bzerangue/osis-bibles)
- The latest build of this plugin
- The latest build of [KuwagataDLL-C++](https://github.com/bbadewa/KuwagataDLL-cpp)

Place the bible's corresponding ``verses.json`` in a hierarchy like this:

```%appdata%/obs-studio/plugin-config/Kuwagata4OBS/bibles/YourBibleShorthandNameHere/verses.json```

Place both KuwagataDLL-C++ and Kuwagata4OBS's DLL files in your OBS Studio install directory's plugins directory.

On Windows, this is typically:
```C:/Program Files/obs-studio/obs-plugins/64bit```

On startup, the plugin should load the alphabetical first bible in your bibles directory. ~~This behaviour can be changed in the plugin settings, and other bibles can be added to the initial load at the cost of program startup time and memory footprint~~. (This behaviour is coming to this plugin soon!)

## Credits
Kuwagata for OBS Studio would not be possible without the use of [open-source software found in the relevant credits section of KuwagataDLL-C++.](https://github.com/bbadewa/KuwagataDLL-cpp)

Credit for OBS-related code belongs (obviously) to Lain Bailey, the Open Broadcaster Software Foundation, and all its contributors. 
