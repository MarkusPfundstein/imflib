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
include src/bin/jp2/CMakeFiles/opj_dump.dir/depend.make

# Include the progress variables for this target.
include src/bin/jp2/CMakeFiles/opj_dump.dir/progress.make

# Include the compile flags for this target's objects.
include src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make

src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.o: src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make
src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.o: ../src/bin/jp2/opj_dump.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/opj_dump.dir/opj_dump.c.o   -c /home/markus/projects/imflib/openjpeg/src/bin/jp2/opj_dump.c

src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/opj_dump.dir/opj_dump.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/bin/jp2/opj_dump.c > CMakeFiles/opj_dump.dir/opj_dump.c.i

src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/opj_dump.dir/opj_dump.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/bin/jp2/opj_dump.c -o CMakeFiles/opj_dump.dir/opj_dump.c.s

src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.o: src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make
src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.o: ../src/bin/jp2/convert.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/opj_dump.dir/convert.c.o   -c /home/markus/projects/imflib/openjpeg/src/bin/jp2/convert.c

src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/opj_dump.dir/convert.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/bin/jp2/convert.c > CMakeFiles/opj_dump.dir/convert.c.i

src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/opj_dump.dir/convert.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/bin/jp2/convert.c -o CMakeFiles/opj_dump.dir/convert.c.s

src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.o: src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make
src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.o: ../src/bin/jp2/index.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/opj_dump.dir/index.c.o   -c /home/markus/projects/imflib/openjpeg/src/bin/jp2/index.c

src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/opj_dump.dir/index.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/bin/jp2/index.c > CMakeFiles/opj_dump.dir/index.c.i

src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/opj_dump.dir/index.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/bin/jp2/index.c -o CMakeFiles/opj_dump.dir/index.c.s

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.o: src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make
src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.o: ../src/bin/common/color.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/opj_dump.dir/__/common/color.c.o   -c /home/markus/projects/imflib/openjpeg/src/bin/common/color.c

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/opj_dump.dir/__/common/color.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/bin/common/color.c > CMakeFiles/opj_dump.dir/__/common/color.c.i

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/opj_dump.dir/__/common/color.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/bin/common/color.c -o CMakeFiles/opj_dump.dir/__/common/color.c.s

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o: src/bin/jp2/CMakeFiles/opj_dump.dir/flags.make
src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o: ../src/bin/common/opj_getopt.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o   -c /home/markus/projects/imflib/openjpeg/src/bin/common/opj_getopt.c

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.i"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/markus/projects/imflib/openjpeg/src/bin/common/opj_getopt.c > CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.i

src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.s"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/markus/projects/imflib/openjpeg/src/bin/common/opj_getopt.c -o CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.s

# Object files for target opj_dump
opj_dump_OBJECTS = \
"CMakeFiles/opj_dump.dir/opj_dump.c.o" \
"CMakeFiles/opj_dump.dir/convert.c.o" \
"CMakeFiles/opj_dump.dir/index.c.o" \
"CMakeFiles/opj_dump.dir/__/common/color.c.o" \
"CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o"

# External object files for target opj_dump
opj_dump_EXTERNAL_OBJECTS =

bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/opj_dump.c.o
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/convert.c.o
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/index.c.o
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/color.c.o
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/__/common/opj_getopt.c.o
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/build.make
bin/opj_dump: bin/libopenjp2.so.2.1.0
bin/opj_dump: src/bin/jp2/CMakeFiles/opj_dump.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/markus/projects/imflib/openjpeg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable ../../../bin/opj_dump"
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/opj_dump.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/bin/jp2/CMakeFiles/opj_dump.dir/build: bin/opj_dump

.PHONY : src/bin/jp2/CMakeFiles/opj_dump.dir/build

src/bin/jp2/CMakeFiles/opj_dump.dir/clean:
	cd /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 && $(CMAKE_COMMAND) -P CMakeFiles/opj_dump.dir/cmake_clean.cmake
.PHONY : src/bin/jp2/CMakeFiles/opj_dump.dir/clean

src/bin/jp2/CMakeFiles/opj_dump.dir/depend:
	cd /home/markus/projects/imflib/openjpeg/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/markus/projects/imflib/openjpeg /home/markus/projects/imflib/openjpeg/src/bin/jp2 /home/markus/projects/imflib/openjpeg/build /home/markus/projects/imflib/openjpeg/build/src/bin/jp2 /home/markus/projects/imflib/openjpeg/build/src/bin/jp2/CMakeFiles/opj_dump.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/bin/jp2/CMakeFiles/opj_dump.dir/depend
