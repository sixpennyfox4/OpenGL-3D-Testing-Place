#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
{
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::UpdateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 proj{ glm::mat4(1.0f) };

	view = glm::lookAt(Position, Position + Orientation, Up);
	proj = glm::perspective(glm::radians(FOVdeg), (float)(width / height), nearPlane, farPlane);

	cameraMatrix = proj * view;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
	int uniformLoc{ glGetUniformLocation(shader.ID, uniform) };
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window, bool mouseInputs, bool mouseEnabled)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Position += speed * Orientation;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Position += speed * -Orientation;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		Position += speed * Up;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		Position += speed * -Up;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 0.4f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		speed = 0.01f;
	}
	else
	{
		speed = 0.1f;
	}


	if (mouseEnabled && mouseInputs)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick)
			{
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double xPos;
			double yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			float rotX{ sensitivity * (float)(yPos - (height / 2)) / height };
			float rotY{ sensitivity * (float)(xPos - (width / 2)) / width };

			glm::vec3 newOrientation{ glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up))) };

			if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				Orientation = newOrientation;
			}

			Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstClick = true;
		}
	}
	else if (mouseInputs)
	{
		if (!glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			firstClick = true;
			return;
		}

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double xPos;
		double yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		float rotX{ sensitivity * (float)(yPos - (height / 2)) / height };
		float rotY{ sensitivity * (float)(xPos - (width / 2)) / width };

		glm::vec3 newOrientation{ glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up))) };

		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
}

void Camera::InputsGame(GLFWwindow* window, bool movementInputs, bool mouseInputs, bool mouseEnabled)
{
	glm::vec3 forward{ glm::normalize(glm::vec3(Orientation.x, 0.0f, Orientation.z)) };
	glm::vec3 right{ glm::normalize(glm::cross(forward, Up)) };

	if (movementInputs)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			Position += speed * forward;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			Position -= speed * forward;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			Position -= speed * right;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			Position += speed * right;
		}
	}


	if (mouseEnabled && mouseInputs)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick)
			{
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double xPos;
			double yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			float rotX{ sensitivity * (float)(yPos - (height / 2)) / height };
			float rotY{ sensitivity * (float)(xPos - (width / 2)) / width };

			glm::vec3 newOrientation{ glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up))) };

			if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				Orientation = newOrientation;
			}

			Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstClick = true;
		}
	}
	else if (mouseInputs)
	{
		if (!glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			firstClick = true;
			return;
		}

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double xPos;
		double yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		float rotX{ sensitivity * (float)(yPos - (height / 2)) / height };
		float rotY{ sensitivity * (float)(xPos - (width / 2)) / width };

		glm::vec3 newOrientation{ glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up))) };

		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
}