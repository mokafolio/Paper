Paper [![Build Status](https://travis-ci.org/mokafolio/Paper.svg?branch=master)](https://travis-ci.org/mokafolio/Paper)
==========

Overview
----------

*Paper* is a C++ port of [paper.js](http://paperjs.org/).

Supported Platforms
----------
*OS X* and *Linux* at this point.

Dependencies
----------

- [CMake](https://cmake.org/) for cross platform building.
- [Stick](https://github.com/mokafolio/Stick) for data structures, containers, allocators etc.
- [Crunch](https://github.com/mokafolio/Crunch) for math.
- [Brick](https://github.com/mokafolio/Brick) for entity/component things that are used to represent the DOM.
- [Scrub](https://github.com/mokafolio/Scrub) for xml/json parsing and composing.


Installation
----------

###RECOMMENDED

Since we are still in pre alpha, the brew tap does not get updated that often, while the master repository will be.
You will most likely get the best experience/stability and features out of simply cloning the current master branch at this point.
If you manually installed the dependencies you can simply build and install *Paper* by doing:

```
mkdir build  
cd build  
cmake ..  
make install
```

If you want to install the dependencies through the git submodules, do this:

```
git submodule init  
git submodule update  
mkdir build  
cd build  
cmake -DBuildSubmodules=On ..  
make install
```


###OS X

The easiest way to install *Paper* on *OS X* is to use [Homebrew](http://brew.sh/).
If you installed *Homebrew*, hook into [this](https://github.com/mokafolio/homebrew-mokatap) custom tap via:
`brew tap mokafolio/mokatap` 
Then run the following to install *Paper* and all its dependencies.
`brew install paper`

Examples
---------
Paper examples are located [here](https://github.com/mokafolio/PaperExamples). A lot more coming soonish.

License
----------
MIT License

Differences to paper.js
----------
Coming soon.

TODO
----------
- make sure the Allocator's are actually used for memory allocation
- add path intersections
- add path splitting
- add boolean operations
- gradients
- different blend modes
- shadows
- A lot more unit tests (specifically for numeric stability).
