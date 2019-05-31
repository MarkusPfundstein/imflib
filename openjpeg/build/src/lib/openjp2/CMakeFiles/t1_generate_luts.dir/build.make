# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/markus/projects/imflib/openjpeg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/markus/projects/imflib/openjpeg/build

# Include any dependencies generated for this target.
include src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/depend.make

# Include the progress variables for this target.
include src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/progress.make

# Include the compile flags for this target's objects.
include src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/flags.make

src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o: src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/flags.make
src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o: ../src/lib/openjp2/t1_generate_luts.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o   -c /home/markus/projects/imflib/openjpeg/src/lib/openjp2/t1_generate_luts.c

src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/lib/openjp2/t1_generate_luts.c > CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.i

src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/lib/openjp2/t1_generate_luts.c -o CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.s

# Object files for target t1_generate_luts
t1_generate_luts_OBJECTS = \
"CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o"

# External object files for target t1_generate_luts
t1_generate_luts_EXTERNAL_OBJECTS =

bin/t1_generate_luts: src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/t1_generate_luts.c.o
bin/t1_generate_luts: src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/build.make
bin/t1_generate_luts: src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../../../bin/t1_generate_luts"
	cd /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/t1_generate_luts.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/build: bin/t1_generate_luts

.PHONY : src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/build

src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/clean:
	cd /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 && $(CMAKE_COMMAND) -P CMakeFiles/t1_generate_luts.dir/cmake_clean.cmake
.PHONY : src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/clean

src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/depend:
	cd /home/markus/projects/imflib/openjpeg/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/markus/projects/imflib/openjpeg /home/markus/projects/imflib/openjpeg/src/lib/openjp2 /home/markus/projects/imflib/openjpeg/build /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2 /home/markus/projects/imflib/openjpeg/build/src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/lib/openjp2/CMakeFiles/t1_generate_luts.dir/depend

