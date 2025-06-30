#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Inc/Camera.h"
#include "Inc/Texture.h"
#include "Inc/OBJ_Loader.hpp"

#include "Inc/Shader.h"
#include "Inc/VAO.h"
#include "Inc/VBO.h"
#include "Inc/EBO.h"

constexpr unsigned int wWidth{ 800 };
constexpr unsigned int wHeight{ 800 };

constexpr float gamma{ 2.2f };

// TODO: Add footstep sound.
// TODO: Add camera movement when walking.
// TODO: Add collision.
// TODO: Add walls and roof.
// TODO: Make the light cube a light bulb.
int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLfloat crosshairVertices[]{
		// Left Triangle
		-0.0025f,  0.0025f, 0.0f,
		 0.0025f,  0.0025f, 0.0f,
		-0.0025f, -0.0025f, 0.0f,

		// Right Triangle
		-0.0025f, -0.0025f, 0.0f,
		 0.0025f,  0.0025f, 0.0f,
		 0.0025f, -0.0025f, 0.0f
	};

	GLuint crosshairIndices[]{
		// Left Triangle
		0, 1, 2,

		// Right Triangle
		3, 4, 5
	};

	GLfloat floorVertices[]{
		//   COORDINATES     |   TEXTURE    |       NORMALS    //
		// Bottom
		-0.5f, -0.02f, 0.5f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, -0.02f, -0.5f,   0.0f, 5.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.02f, -0.5f,    5.0f, 5.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.02f, 0.5f,     5.0f, 0.0f,		0.0f, -1.0f, 0.0f,

		// Top
		-0.5f, 0.02f, 0.5f,	    0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		-0.5f, 0.02f, -0.5f,    0.0f, 5.0f,		0.0f, 1.0f, 0.0f,
		0.5f, 0.02f, -0.5f,     5.0f, 5.0f,		0.0f, 1.0f, 0.0f,
		0.5f, 0.02f, 0.5f,      5.0f, 0.0f,		0.0f, 1.0f, 0.0f
	};

	GLuint floorIndices[]{
		// Bottom
		0, 1, 2,
		0, 2, 3,

		// Top
		4, 5, 6,
		4, 6, 7,

		// Front
		0, 3, 4,
		3, 4, 7,

		// Back
		1, 2, 5,
		2, 5, 6,

		// Left
		0, 1, 4,
		1, 4, 5,

		// Right
		3, 2, 7,
		2, 7, 6
	};

	GLfloat lightVertices[]{
		-0.1f, -0.1f,  0.1f,
		-0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f,  0.1f,
		-0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f,  0.1f
	};

	GLuint lightIndices[]{
		0, 1, 2,
		0, 2, 3,
		0, 4, 7,
		0, 7, 3,
		3, 7, 6,
		3, 6, 2,
		2, 6, 5,
		2, 5, 1,
		1, 5, 4,
		1, 4, 0,
		4, 5, 6,
		4, 6, 7
	};

	GLenum polygonMode{ GL_FILL };

	bool falling{ false };
	bool freecam{ false };

	float floorHalfExtent{ (0.5f * 5.0f) + 0.06f };

	GLFWwindow* window{ glfwCreateWindow(wWidth, wHeight, "3D Testing", NULL, NULL) };
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window.\n";

		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to init GLAD.\n";

		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	glViewport(0, 0, wWidth, wHeight);

	objl::Loader tableLoader;
	if (!tableLoader.LoadFile("Assets/table.obj"))
	{
		std::cerr << "Failed to load table.obj!\n";
	}

	VAO tableVAO;
	VBO tableVBO;
	EBO tableEBO;
	size_t tableIndexCount{ 0 };

	if (!tableLoader.LoadedMeshes.empty())
	{
		objl::Mesh& mesh{ tableLoader.LoadedMeshes[0] }; // Table is in 1 mesh
		tableIndexCount = mesh.Indices.size();

		std::vector<GLfloat> verts;
		verts.reserve(mesh.Vertices.size() * 5);

		for (objl::Vertex& v : mesh.Vertices)
		{
			verts.push_back(v.Position.X);
			verts.push_back(v.Position.Y);
			verts.push_back(v.Position.Z);

			verts.push_back(v.Position.X * 2.5f);
			verts.push_back(v.Position.Z * 2.5f);

			verts.push_back(v.Normal.X);
			verts.push_back(v.Normal.Y);
			verts.push_back(v.Normal.Z);
		}

		tableVAO.Bind();
		tableVBO.setup(verts.data(), verts.size() * sizeof(GLfloat));
		tableEBO.setup(mesh.Indices.data(), mesh.Indices.size() * sizeof(GLuint));

		tableVAO.LinkAttrib(tableVBO, 0, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)0);
		tableVAO.LinkAttrib(tableVBO, 1, 2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
		tableVAO.LinkAttrib(tableVBO, 2, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));

		tableVAO.Unbind();
		tableVBO.Unbind();
		tableEBO.Unbind();
	}

	objl::Loader chairLoader;
	if (!chairLoader.LoadFile("Assets/chair.obj"))
	{
		std::cerr << "Failed to load chair.obj!\n";
	}

	VAO chairVAO;
	VBO chairVBO;
	EBO chairEBO;
	size_t chairIndexCount{ 0 };

	if (!chairLoader.LoadedMeshes.empty())
	{
		objl::Mesh& mesh{ chairLoader.LoadedMeshes[0] }; // Chair is in 1 mesh
		chairIndexCount = mesh.Indices.size();

		std::vector<GLfloat> verts;
		verts.reserve(mesh.Vertices.size() * 5);

		for (objl::Vertex& v : mesh.Vertices)
		{
			verts.push_back(v.Position.X);
			verts.push_back(v.Position.Y);
			verts.push_back(v.Position.Z);

			verts.push_back(v.Position.X * 2.5f);
			verts.push_back(v.Position.Z * 2.5f);

			verts.push_back(v.Normal.X);
			verts.push_back(v.Normal.Y);
			verts.push_back(v.Normal.Z);
		}

		chairVAO.Bind();
		chairVBO.setup(verts.data(), verts.size() * sizeof(GLfloat));
		chairEBO.setup(mesh.Indices.data(), mesh.Indices.size() * sizeof(GLuint));

		chairVAO.LinkAttrib(chairVBO, 0, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)0);
		chairVAO.LinkAttrib(chairVBO, 1, 2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
		chairVAO.LinkAttrib(chairVBO, 2, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));

		chairVAO.Unbind();
		chairVBO.Unbind();
		chairEBO.Unbind();
	}

	objl::Loader carpetLoader;
	if (!carpetLoader.LoadFile("Assets/carpet.obj"))
	{
		std::cerr << "Failed to load carpet.obj!\n";
	}

	VAO carpetVAO;
	VBO carpetVBO;
	EBO carpetEBO;
	size_t carpetIndexCount{ 0 };

	if (!carpetLoader.LoadedMeshes.empty())
	{
		objl::Mesh& mesh{ carpetLoader.LoadedMeshes[0] }; // Carpet is in 1 mesh
		carpetIndexCount = mesh.Indices.size();

		std::vector<GLfloat> verts;
		verts.reserve(mesh.Vertices.size() * 5);

		for (objl::Vertex& v : mesh.Vertices)
		{
			verts.push_back(v.Position.X);
			verts.push_back(v.Position.Y);
			verts.push_back(v.Position.Z);

			verts.push_back(v.Position.X * 0.6f);
			verts.push_back(v.Position.Z * 0.6f);

			verts.push_back(v.Normal.X);
			verts.push_back(v.Normal.Y);
			verts.push_back(v.Normal.Z);
		}

		carpetVAO.Bind();
		carpetVBO.setup(verts.data(), verts.size() * sizeof(GLfloat));
		carpetEBO.setup(mesh.Indices.data(), mesh.Indices.size() * sizeof(GLuint));

		carpetVAO.LinkAttrib(carpetVBO, 0, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)0);
		carpetVAO.LinkAttrib(carpetVBO, 1, 2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
		carpetVAO.LinkAttrib(carpetVBO, 2, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));

		carpetVAO.Unbind();
		carpetVBO.Unbind();
		carpetEBO.Unbind();
	}

	Shader crosshairShader(get_file_contents("Shaders/3D Testing/crosshair.vert").c_str(), get_file_contents("Shaders/3D Testing/crosshair.frag").c_str());
	Shader floorShader(get_file_contents("Shaders/3D Testing/floor.vert").c_str(), get_file_contents("Shaders/3D Testing/floor.frag").c_str());
	Shader tableShader{ get_file_contents("Shaders/3D Testing/table.vert").c_str(), get_file_contents("Shaders/3D Testing/table.frag").c_str() };
	Shader chairShader{ get_file_contents("Shaders/3D Testing/chair.vert").c_str(), get_file_contents("Shaders/3D Testing/chair.frag").c_str() };
	Shader carpetShader{ get_file_contents("Shaders/3D Testing/carpet.vert").c_str(), get_file_contents("Shaders/3D Testing/carpet.frag").c_str() };
	Shader lightShader{ get_file_contents("Shaders/3D Testing/light.vert").c_str(), get_file_contents("Shaders/3D Testing/light.frag").c_str() };

	VAO crosshairVAO;
	crosshairVAO.Bind();

	VBO crosshairVBO(crosshairVertices, sizeof(crosshairVertices));
	EBO crosshairEBO(crosshairIndices, sizeof(crosshairIndices));

	crosshairVAO.LinkAttrib(crosshairVBO, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);
	crosshairVAO.Unbind();
	crosshairVBO.Unbind();
	crosshairEBO.Unbind();

	VAO floorVAO;
	floorVAO.Bind();

	VBO floorVBO(floorVertices, sizeof(floorVertices));
	EBO floorEBO(floorIndices, sizeof(floorIndices));

	floorVAO.LinkAttrib(floorVBO, 0, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)0);
	floorVAO.LinkAttrib(floorVBO, 1, 2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	floorVAO.LinkAttrib(floorVBO, 2, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
	floorVAO.Unbind();
	floorVBO.Unbind();
	floorEBO.Unbind();

	VAO lightVAO;
	lightVAO.Bind();

	VBO lightVBO(lightVertices, sizeof(lightVertices));
	EBO lightEBO(lightIndices, sizeof(lightIndices));

	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();

	//glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClearColor(pow(0.07f, gamma), pow(0.13f, gamma), pow(0.17f, gamma), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glfwSwapBuffers(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glm::vec4 lightColor{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

	glm::vec3 lightPos{ glm::vec3(2.0f, 1.2f, 0.0f) };
	glm::mat4 lightModel{ glm::mat4(1.0f) };
	lightModel = glm::translate(lightModel, lightPos);

	glm::vec3 floorPos{ glm::vec3(0.0f, 0.0f, 0.0f) };
	glm::mat4 floorModel{ glm::mat4(1.0f) };
	floorModel = glm::translate(floorModel, floorPos);
	floorModel = glm::scale(floorModel, glm::vec3(5.0f, 1.0f, 5.0f));

	glm::vec3 tablePos{ glm::vec3(2.0f, 0.0f, 0.0f) };
	glm::mat4 tableModel{ glm::mat4(1.0f) };
	tableModel = glm::translate(tableModel, tablePos);
	tableModel = glm::scale(tableModel, glm::vec3(0.2f, 0.2f, 0.2f));

	glm::vec3 chairPos{ glm::vec3(1.5f, 0.0f, 0.0f) };
	glm::mat4 chairModel{ glm::mat4(1.0f) };
	chairModel = glm::translate(chairModel, chairPos);
	chairModel = glm::scale(chairModel, glm::vec3(0.16f, 0.16f, 0.16f));

	glm::vec3 carpetPos{ glm::vec3(0.0f, 0.02f, 0.0f) };
	glm::mat4 carpetModel{ glm::mat4(1.0f) };
	carpetModel = glm::translate(carpetModel, carpetPos);
	carpetModel = glm::scale(carpetModel, glm::vec3(0.2f, 0.16f, 0.2f));

	Texture floorWoodTex("Assets/wood tex3.png", GL_TEXTURE_2D, 0, GL_LINEAR, GL_MIRRORED_REPEAT);
	floorWoodTex.texUnit(floorShader, "tex0", 0);

	Texture tableWoodTex("Assets/wood tex.png", GL_TEXTURE_2D, 0, GL_LINEAR, GL_MIRRORED_REPEAT);
	tableWoodTex.texUnit(tableShader, "tex0", 0);

	Texture chairWoodTex("Assets/wood tex2.png", GL_TEXTURE_2D, 0, GL_LINEAR, GL_MIRRORED_REPEAT);
	chairWoodTex.texUnit(chairShader, "tex0", 0);

	Texture carpetTex("Assets/carpet texture.png", GL_TEXTURE_2D, 0, GL_LINEAR, GL_MIRRORED_REPEAT);
	carpetTex.texUnit(carpetShader, "tex0", 0);

	Texture floorWoodTexSpec("Assets/wood tex3 specular.png", GL_TEXTURE_2D, 1, GL_LINEAR, GL_MIRRORED_REPEAT);
	floorWoodTexSpec.texUnit(floorShader, "tex1", 1);

	Texture tableWoodTexSpec("Assets/wood tex specular.png", GL_TEXTURE_2D, 1, GL_LINEAR, GL_MIRRORED_REPEAT);
	tableWoodTexSpec.texUnit(tableShader, "tex1", 1);

	Texture chairWoodTexSpec("Assets/wood tex2 specular.png", GL_TEXTURE_2D, 1, GL_LINEAR, GL_MIRRORED_REPEAT);
	chairWoodTexSpec.texUnit(chairShader, "tex1", 1);

	Texture carpetTexSpec("Assets/carpet texture specular.png", GL_TEXTURE_2D, 1, GL_LINEAR, GL_MIRRORED_REPEAT);
	carpetTexSpec.texUnit(carpetShader, "tex1", 1);

	Camera cam(wWidth, wHeight, glm::vec3(0.0f, 1.0f, 0.0f));
	cam.speed = 0.02f;

	double prevTime{ 0.0 };
	double crntTime{ 0.0 };
	double timeDiff;
	unsigned int fpsCounter{ 0 };

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& imIO{ ImGui::GetIO() }; (void)imIO;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	while (!glfwWindowShouldClose(window))
	{
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		fpsCounter++;
		if (timeDiff >= 1.0 / 30.0)
		{
			std::string fps{ std::to_string(static_cast<int>((1.0 / timeDiff) * fpsCounter)) };
			std::string newTitle{ "3D Testing - " + fps + "FPS" };

			glfwSetWindowTitle(window, newTitle.c_str());
			prevTime = crntTime;
			fpsCounter = 0;
		}

		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug Menu");
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) cam.firstClick = true;

		if (!ImGui::IsWindowFocused())
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !falling)
			{
				cam.speed = 0.04f;
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !falling)
			{
				cam.speed = 0.008f;
				//cam.Position.y = 0.5f;
			}
			else
			{
				cam.speed = 0.02f;
				//cam.Position.y = 1.0f;
			}
		}

		if (!falling)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && (!ImGui::IsWindowFocused()))
			{
				if (!freecam) cam.Position.y = 0.5f;
			}
			else
			{
				if (!freecam) cam.Position.y = 1.0f;
			}
		}
		else
		{
			cam.Position.y -= 0.04f;
		}

		if ((cam.Position.x < -floorHalfExtent || cam.Position.x > floorHalfExtent ||
			cam.Position.z < -floorHalfExtent || cam.Position.z > floorHalfExtent) && !falling && !freecam)
		{
			falling = true;
		}

		glClearColor(pow(0.07f, gamma), pow(0.13f, gamma), pow(0.17f, gamma), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!ImGui::IsWindowFocused())
		{
			(!freecam) ? cam.InputsGame(window, !falling, !(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)) : cam.Inputs(window, !(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS));
		}
		else
		{
			cam.firstClick = true;
		}
		cam.UpdateMatrix(45.0f, 0.1f, 100.0f);

		// Draw light cube

		lightShader.Activate();
		lightVAO.Bind();

		cam.Matrix(lightShader, "camMatrix");

		glUniformMatrix4fv(lightShader.GetUniformLoc("lightModel"), 1, GL_FALSE, glm::value_ptr(lightModel));

		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		// Draw floor

		floorShader.Activate();
		floorWoodTex.Bind();
		floorWoodTexSpec.Bind();
		floorVAO.Bind();

		cam.Matrix(floorShader, "camMatrix");

		glUniformMatrix4fv(floorShader.GetUniformLoc("floorModel"), 1, GL_FALSE, glm::value_ptr(floorModel));
		glUniform4f(floorShader.GetUniformLoc("lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(floorShader.GetUniformLoc("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(floorShader.GetUniformLoc("camPos"), cam.Position.x, cam.Position.y, cam.Position.z);

		glDrawElements(GL_TRIANGLES, sizeof(floorIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		lightShader.Activate();
		glUniformMatrix4fv(lightShader.GetUniformLoc("lightModel"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniform4f(lightShader.GetUniformLoc("lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

		// Draw table

		tableShader.Activate();
		tableWoodTex.Bind();
		tableWoodTexSpec.Bind();
		tableVAO.Bind();

		cam.Matrix(tableShader, "camMatrix");

		glUniformMatrix4fv(tableShader.GetUniformLoc("tableModel"), 1, GL_FALSE, glm::value_ptr(tableModel));
		glUniform4f(tableShader.GetUniformLoc("lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(tableShader.GetUniformLoc("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(tableShader.GetUniformLoc("camPos"), cam.Position.x, cam.Position.y, cam.Position.z);

		glDrawElements(GL_TRIANGLES, (GLsizei)tableIndexCount, GL_UNSIGNED_INT, 0);

		// Draw chairs

		chairShader.Activate();
		chairWoodTex.Bind();
		chairWoodTexSpec.Bind();
		chairVAO.Bind();

		cam.Matrix(chairShader, "camMatrix");

		glUniformMatrix4fv(chairShader.GetUniformLoc("chairModel"), 1, GL_FALSE, glm::value_ptr(chairModel));
		glUniform4f(chairShader.GetUniformLoc("lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(chairShader.GetUniformLoc("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(chairShader.GetUniformLoc("camPos"), cam.Position.x, cam.Position.y, cam.Position.z);
		glUniformMatrix3fv(chairShader.GetUniformLoc("normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(chairModel)))));

		glDrawElements(GL_TRIANGLES, (GLsizeiptr)chairIndexCount, GL_UNSIGNED_INT, 0);

		chairModel = glm::mat4(1.0f);
		chairPos = glm::vec3(2.0f, 0.0f, 0.5f);
		chairModel = glm::translate(chairModel, chairPos);
		chairModel = glm::scale(chairModel, glm::vec3(0.16f, 0.16f, 0.16f));
		chairModel = glm::rotate(chairModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(chairShader.GetUniformLoc("chairModel"), 1, GL_FALSE, glm::value_ptr(chairModel));
		glUniformMatrix3fv(chairShader.GetUniformLoc("normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(chairModel)))));

		glDrawElements(GL_TRIANGLES, (GLsizeiptr)chairIndexCount, GL_UNSIGNED_INT, 0);

		chairModel = glm::mat4(1.0f);
		chairPos = glm::vec3(2.0f, 0.0f, -0.5f);
		chairModel = glm::translate(chairModel, chairPos);
		chairModel = glm::scale(chairModel, glm::vec3(0.16f, 0.16f, 0.16f));
		chairModel = glm::rotate(chairModel, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(chairShader.GetUniformLoc("chairModel"), 1, GL_FALSE, glm::value_ptr(chairModel));
		glUniformMatrix3fv(chairShader.GetUniformLoc("normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(chairModel)))));

		glDrawElements(GL_TRIANGLES, (GLsizeiptr)chairIndexCount, GL_UNSIGNED_INT, 0);

		chairModel = glm::mat4(1.0f);
		chairPos = glm::vec3(1.5f, 0.0f, 0.0f);
		chairModel = glm::translate(chairModel, chairPos);
		chairModel = glm::scale(chairModel, glm::vec3(0.16f, 0.16f, 0.16f));

		// Draw carpet

		carpetShader.Activate();
		carpetTex.Bind();
		carpetTexSpec.Bind();
		carpetVAO.Bind();

		cam.Matrix(carpetShader, "camMatrix");

		glUniformMatrix4fv(carpetShader.GetUniformLoc("carpetModel"), 1, GL_FALSE, glm::value_ptr(carpetModel));
		glUniform4f(carpetShader.GetUniformLoc("lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(carpetShader.GetUniformLoc("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(carpetShader.GetUniformLoc("camPos"), cam.Position.x, cam.Position.y, cam.Position.z);

		glDrawElements(GL_TRIANGLES, (GLsizeiptr)carpetIndexCount, GL_UNSIGNED_INT, 0);

		// Draw crosshair

		crosshairShader.Activate();
		crosshairVAO.Bind();

		glDrawElements(GL_TRIANGLES, sizeof(crosshairIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		
		ImGui::SetWindowSize(ImVec2{ 250, 300 });

		ImGui::Text("            -General-");

		if (ImGui::Button("Reset"))
		{
			falling = false;

			cam.Position = glm::vec3(0.0f, 1.0f, 0.0f);
			cam.Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
			cam.Up = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		ImGui::Checkbox("Free Camera", &freecam);

		ImGui::Text("             -Draw-");

		if (ImGui::Button("Toggle Wireframe Mode"))
		{
			polygonMode = (polygonMode == GL_LINE) ? GL_FILL : GL_LINE;
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	crosshairVAO.Delete();
	crosshairVBO.Delete();
	crosshairEBO.Delete();
	crosshairShader.Delete();

	floorVAO.Delete();
	floorVBO.Delete();
	floorEBO.Delete();
	floorShader.Delete();
	floorWoodTex.Delete();
	floorWoodTexSpec.Delete();

	tableVAO.Delete();
	tableVBO.Delete();
	tableEBO.Delete();
	tableShader.Delete();
	tableWoodTex.Delete();
	tableWoodTexSpec.Delete();

	chairVAO.Delete();
	chairVBO.Delete();
	chairEBO.Delete();
	chairShader.Delete();
	chairWoodTex.Delete();
	chairWoodTexSpec.Delete();

	carpetVAO.Delete();
	carpetVBO.Delete();
	carpetEBO.Delete();
	carpetShader.Delete();
	carpetTex.Delete();
	carpetTexSpec.Delete();

	lightVAO.Delete();
	lightVBO.Delete();
	lightEBO.Delete();
	lightShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}