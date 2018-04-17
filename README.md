# VkBindingBenchmark
Using Sponza to test performance cost of resource binding strategies in Vulkan

Note: The Amazon Bistro scene was too big to feasibly store in git, so I've instead committed a zip file called "bistro.7z" in the data/meshes folder that contains the interior and exterior meshes. You'll need to unzip those before running the program if you want to test the bistro scene. 

The Bistro Files were made public by Amazon and NVidia: https://developer.nvidia.com/orca/amazon-lumberyard-bistro
The Crytek Sponza files were made public by (no surprise) Crytek: http://www.crytek.com/cryengine/cryengine3/downloads

# In this branch
The cmake-linux branch is meant to port all of the previous code to a CMake build environment in order to run the tests on more than just Windows.

SDL2 is now a dependency, and you will need to download the Development Libraries from here: https://www.libsdl.org/download-2.0.php
If you're running Windows, make sure you set up your environment variables such that "SDL2" points to wherever you put the SDL2-X.X.X directory, where the X's are the version numbers.

In addition, you will need to download and install a Vulkan SDK, for example, the LunarG Vulkan SDK: https://vulkan.lunarg.com/
In the case of the LunarG Vulkan SDK, you will run an installer that should set up the environment variables for you.

Also, git submodules are now used for a few of the external dependencies, so when you clone, either use "git clone --recursive" or run "git submodule init" followed by "git submodule update" after cloning.

Significant changes ended up being made in the top-level organization of the code, despite the intention of performing a simple port to CMake. Some new data structures were introduced, and dependencies between files and structures got a little bit more complex, unfortunately. Some of this was a result of the move to SDL2, and some of it had to do with more trivial concerns, such as handling window resizes, which need to be handled if you want to, say, run the benchmark in a tiling window manager.

In case this helps anyone, a CMake-based project like this can still be opened in Visual Studio 2017, without even having to generate a VS solution or anything. Just use File > Open > Folder on the root directory of the project and VS2017 should detect and run the CMakeLists.txt file.
