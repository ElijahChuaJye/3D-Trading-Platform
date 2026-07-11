==============================================================================
BLITZ 3D ENGINE - COMMAND LINE BUILD AND RUN GUIDE
==============================================================================

[REMINDER LOGIC]
* Use 1 dash "-" for single-letter flags (e.g., -S, -B).
* Use 2 dashes "--" for full words (e.g., --build, --config).

------------------------------------------------------------------------------
WAY 1: THE FULL CLI ROUTE (Fastest way via Command Prompt)
------------------------------------------------------------------------------
Run these commands from the project root folder:

1. Generate the Project Structure:
   cmake -S . -B build
   (-S . means source files are here; -B build means output compilation files to a folder named "build"). [cite: 13, 16]

2. Compile the Engine and App Targets:
   cmake --build build --config Debug
   (--build starts compiling the files inside the target folder; --config Debug keeps validation layers active). [cite: 14]

3. Run the App:
   .\build\App\Debug\AppExecutable.exe

------------------------------------------------------------------------------
WAY 2: THE HYBRID VISUAL STUDIO ROUTE
------------------------------------------------------------------------------
1. Generate the Project files:
   cmake -S . -B build 

2. Navigate into your new physical "build" folder using Windows File Explorer.

3. Double-click "TradingPlatform3D.sln" to open the entire project in Visual Studio.
   * Right-click "AppExecutable" in the Solution Explorer and click "Set as Startup Project".
   * Press F5 to run!

------------------------------------------------------------------------------
MAINTENANCE UTILITIES
------------------------------------------------------------------------------
To completely delete the build folder and force a 100% fresh clean compile:
rmdir /s /q build && cmake -S . -B build && cmake --build build --config Debug

Vulkan SDK Toolkit Download Link:
https://vulkan.lunarg.com/
==============================================================================