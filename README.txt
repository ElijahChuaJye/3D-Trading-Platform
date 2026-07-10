2 Ways to build 

Fastest way
User command prompt in root folder
"-S ." meaning source is here and -B build means put all the fild to build 
cmake -S . -B <folder_name>
"--build" start building now, build refers to folder name. 1 "-" for single letter, 2 "-" for words
cmake --build <folder_name>

Hybrid way
cmake -S . -B build (Run it once)
Go to build folder and I will see a file named TradingPlatform3D.sln

To delete and rebuild in one command:
rmdir /s /q <folder_name>(This deletes that folder so you start fresh).

To download vulkan toolkit:
https://vulkan.lunarg.com/