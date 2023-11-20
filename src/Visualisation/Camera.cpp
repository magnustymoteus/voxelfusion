#include "Camera.h"
//based upon youtube.com/watch?v=86_pQCKOIPk

Camera::Camera(int width, int height, glm::vec3 position) : width(width), height(height), Position(position)
{
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform)
{
	glm::mat4 view;
	glm::mat4 projection;

	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(FOVdeg), (float)(width / height), nearPlane, farPlane);

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));

}

void Camera::Inputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Position += speed * Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Position += speed * -Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, width / 2, height / 2);
			firstClick = false;

		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		float rotationX = sensitivity * (float)(mouseY - height / 2) / height;
		float rotationY = sensitivity * (float)(mouseX - width / 2) / width;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotationX), glm::normalize(glm::cross(Orientation, Up)));
		Orientation = newOrientation;

		Orientation = glm::rotate(Orientation, glm::radians(-rotationY), Up);
		glfwSetCursorPos(window, width / 2, height / 2);

	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}
