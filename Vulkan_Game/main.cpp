#include "display.h"
#include <GLFW/glfw3.h>
#include "scene.h"

int main() {
	Display display;
	Scene* scene = new Scene();
	while (!glfwWindowShouldClose(display.window))
	{
		glfwPollEvents();
		display.render(scene);
	}
	return 0;
}