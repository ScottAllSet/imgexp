imgexp - A lightweight, extremely fast library that lets you search for patterns 
in images using expressions.

=======================================================
Copyright 2013 - Scott R. Jones (sjones204g@gmail.com)

=======================================================
ThirdParty dependencies

imgexp requires:
G++ >= 4.6
Boost >= 1.55.0 (headers only)
JsonCpp >= 0.6.0-rc2
cmake >= 2.8.7 (maybe 2.8 if you're not building expbuilder)

Additionally, imgexptest requires:
Google Test >= 1.7.0

Additionally, expbuilder requires:
Qt >= 5.2

imgexp, imgexptest, and expbuilder require environment variables that point to their dependencies:
BOOST_DIR - (E.g. C:\boost_1_55_0)
JSONCPP_DIR - (E.g. C:\JsonCpp_0.6.0-rc2)
GTEST_DIR - (E.g. C:\gtest-1.7.0)
QTDIR (note the lack of _) - (E.g. C:\qt-everywhere-opensource-src-5.2.0)

================================================
Build instructions (substitute C:\dev with your workspace)
On Windows using MinGW:

To build imgexp (the library):

cd c:\dev\imgexp
mkdir build && cd build
cmake -G"MinGW Makefiles" ..
mingw32-make.exe

To build imgexptest (the tests for the library):

cd c:\dev\imgexp\test
mkdir build && cd build
cmake -G"MinGW Makefiles" ..
mingw32-make.exe

To build expbuilder (the expression builder GUI):

cd c:\dev\imgexp\expbuilder
mkdir build && cd build
cmake -G"MinGW Makefiles" ..
mingw32-make.exe

============
Linux:

Do the same as for Windows except use this cmake command:
cmake -G"Unix Makefiles" ..

And this make command:
make

============
Visual C++ 2012/2013:

Do the same as for MinGW except use this cmake command:
For VS 2012:
cmake -G"Visual Studio 11 2012" ..

For VS 2013:
cmake -G"Visual Studio 12 2013" ..

Use this make command for all Visual Studio versions:
nmake



