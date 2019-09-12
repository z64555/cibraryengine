# cibraryengine
A game engine, presumably.

Third-party Dependencies:
  Boost, OpenAL, OpenGL, SOIL
  
Supported Platforms:
  * Windows
  If you want to work on Linux or MacOS (or any other platform), you'll have to do it yourself since we currently don't have anyone
  trained with those systems.  Please extend the CMake files instead of using your own build system.

Getting Started:
  CMake should be doing most of the footwork to get a project file/solution for you, but there's still a few things you need to do
  before you can run from debug.

  OpenGL Headers
    Double-check that the OPENGL_INCLUDE path is valid and set to a location that has the following headers:
  
    gl.h
    gl3.h
    glext.h
    glu.h

  SOIL
    You need to have libSOIL built by your compiler of choice and saved as a .lib.  The MSVC projects its SDK provides will generate
    a SOIL.lib, so just rename it to libSOIL.lib and you'll be set.

Debugging:
  If you want to use an IDE debugger, such as the one in Visual Studio, you have to set the working directory within the TestProject's
  property pages to the source code root directory. This is so that the .exe can find runtime assets
  
  For Visual Studio, you can try using $(ProjectDir)..\..\..\
  
  You may also need to copy lua51.dll and lua5.1.dll to be with the compiled .exe.
