CC		:= clang
CFLAGS	:= -std=c99 -Os
CFLAGS	+= -Wextra -Wall -Werror
CFLAGS	+= -Wfloat-equal -Winit-self
CFLAGS	+= -Wundef -Wshadow -Wpointer-arith
CFLAGS	+= -Wcast-align -Wstrict-prototypes
#CFLAGS	+= -Wstrict-overlfow=5 -Waggregate-return
CFLAGS	+= -Wwrite-strings -Wcast-qual
CFLAGS	+= -Wswitch-default -Wswitch-enum
CFLAGS	+= -Wconversion -Wunreachable-code

CFLAGS	+= -Ilib/glad/include -Ilib/glfw/include
LDFLAGS	:= lib/glad/src/glad.o lib/glfw/src/libglfw3.a

LDFLAGS	+= -framework OpengL -framework IOKit -framework CoreVideo -framework Cocoa

GLAD := lib/glad/src/glad.o
GLFW := lib/glfw/src/libglfw3.a

.PHONY: all main

all: libs main

libs: $(GLAD) $(GLFW)

$(GLFW): $(wildcard lib/glfw/src/*.c lib/glfw/src/*.m)
	@echo compiling glfw
	@cd lib/glfw && \
	cmake . -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" && \
	make

$(GLAD): lib/glad/src/gl.c
	@echo compiling glad
	@cd lib/glad && \
	$(CC) -o src/glad.o -Iinclude -c src/gl.c

main: src/main.c
	@echo compiling main
	@$(CC) -o main $(CFLAGS) $(LDFLAGS) src/main.c