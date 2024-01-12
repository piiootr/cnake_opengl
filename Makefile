GLFW := lib/glfw/src/libglfw3.a

glfw:
	cd lib/glfw && \\
	cmake . -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && \\
	make
