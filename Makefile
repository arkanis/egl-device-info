# make -p lists all defined variables and implicit rules. Useful to see what we start out with.
# $(warning $(VAR)) is usefull to output the value of a variable for debugging.

# Setup implicit rule to build object files
CC       = gcc
CPPFLAGS = -Iinclude
CFLAGS   = -std=c99 -Werror -Wall -Wextra -Wno-unused-parameter -g

# Applications, object files are created by implicit rules
egl-device-info: LDLIBS += -l:libEGL.so.1 -l:libOpenGL.so.0 -ldl
