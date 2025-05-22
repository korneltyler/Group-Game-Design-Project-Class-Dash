# CSCI437 Group 1 Project
Class Dash is a side-scrolling platformer where the goal is for the student to make it to their final exams on time. It has many obstacles and distractions that the player must avoid, as well as collectibles that can help them out. The game takes you through many of the locations at the College of William & Mary, making it especially interesting for current or former students.

## Plot Overview
You are a senior at the College of William & Mary entering your last week of finals. In order to graduate, you must make it to all of your finals on time! At the start of each day, you will see your exam start time and a clock will display your current time as you dash to class. While traversing various campus locations such as Swem, The Sunken Gardens, and Crim Dell, you must avoid distractions and obstacles that can slow you down. Occupational hazards like puddles, construction, and loose bricks can slow your movements and people can shoot sound waves at you that will slow you down and "engage in conversation". You must work to avoid these obstacles by moving left and right, jumping, and “shooting down” people with your projectiles that tell them you can’t talk right now. If you make it to your class before the given exam start time, you pass that level and can move on to the next day. After completing your Monday-Friday exams, you will finally graduate.

## How to Run

The code package relies on CMake, a cross platform makefile generator. There exist versions of
CMake for windows, linux, and OSX. While generally each version behaves similarly, depending on
which C++ compiler you have you might need to add additional parameters. 

**For a linux system with a standard gnu c/c++ compiler:**

First, clone the repository to your local machine. 
1. make a subdirectory (under CSCI437-Spring25-Group1) named 'Release': mkdir Release
2. enter the subdir: cd Release
3. and generate the makefiles: cmake .. (mind the ‘..’)

**If on Windows:**

It is highly recommended to us linux (or OSX) if available on your machine. However, in case you
only have a Windows install, we recommend MSYS2 and mingw64. It is recommended if you have an
older MSYS2 install to remove it first and start with a clean version.
1) Install MSYS2: https://www.msys2.org/
2) Open the MSYS2 MINGW64 terminal (not the MSYS2 UCRT64 terminal)
2) Install mingw64: pacman -S mingw-w64-x86_64-toolchain (install all; 800Mb)
3) Install cmake: pacman -S mingw-w64-x86_64-cmake
4) Install git: pacman -S git
5) Optional: add <msys2 install path>/mingw64/bin to your path if you also want to be able to run
your code by double clicking. To add to your path, open "Edit environment variables" -> select
Path -> Edit -> New -> Browse
We will use the MSYS2 MINGW64 terminal for **all** steps.
1) To access your C-drive use: cd /c/
2) Next, git clone the code to your location of choice and use cd /c/<path> to
enter the location in MSYS2. If you get an error that git does not support 'https', then you probably
copy-pasted an invisible character before https; just delete all spaces between 'clone' and 'https',
and re-add a new space; this will delete the invisible character.
3) Make a directory 'Release' mkdir Release
2) Next, generate make files (capitalization of “MinGW Makefiles” matters!):
cd Debug
cmake.exe -G "MinGW Makefiles" ..
3) The above step should complete without an error and find a C++ compiler. Next we can compile
the code: mingw32-make.exe
4) We can now run the game: ./classdash.exe

## How to Play
**Character Movements:**

WASD or arrow keys control directional movement

 A/left arrow and D/right arrow to move

 W/up to jump, space to use projectile ability


**Menu and Navigation**

Navigate menu items with WASD or arrow keys and enter to select

Escape to pause the game.

**GamePlay**

Navigate through each level using the character movements described above in order to reach the end before the time runs out.

Avoid running into enemies (other students), as they will slow your movements. If you physically run into an enemy, you will recieve a time deduction from your remaining game time. If you are in range of some enemies, they will try to talk to you using their sound wave projectile. If hit, you will slow down for a few seconds.

Defend yourself from enemies by pressing SPACE to shoot a soundwave projectile at them. Two hits and the enemy will despawn.

Occupational hazards will also slow you down. Stepping in puddles or tripping on loose bricks will similarly reduce your movement speed temporarily.

Pick up coffee cups on your way to class, as the caffeine will give you a temporay speed boost!

Some levels have potholes or ditches that you can fall in. Falling will result in a time deduction from your remaining time.

# Credits

This game was made as a result of a team effort of 4 students during a 2-month agile development period for the Game Design course at William & Mary in Spring 2025. 

**Developers:** Cassie Baker, Miles Saunders, Sam Brothers, Kornel Tyler

## My Contributions
1) Created initial level designs
2) Created sprites and tiles for game
3) Created projectile class
4) Created intial sprite and spritesheet classes
5) Created and integrated Timer Class
6) Created and integrated Enemy Class
7) Create initila enemy projectiles
8) Fix enemy player tracking bugs
