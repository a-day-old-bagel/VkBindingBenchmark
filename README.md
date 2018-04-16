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

In case you were wondering, a CMake-based project like this can still be used in Visual Studio, at least if you're running the 2017 version. Just use File > Open > Folder on the root directory of the project and VS2017 should detect and run the CMakeLists.txt file.
