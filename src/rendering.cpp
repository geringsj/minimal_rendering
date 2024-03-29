// adapted from https://www.glfw.org/docs/latest/quick_guide.html

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_EXPLICIT_CTOR
#include <glm/glm.hpp> // vec2, vec3, mat4, radians
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/transform.hpp> // rotate in degrees around axis

#include <iostream>
#include <vector>

struct Vertex
{
	float x, y, z;
	float r, g, b;
};

struct RenderVertices {
	GLuint vertex_array = 0, vertex_buffer = 0;
	GLuint element_array = 0;

	void init(const std::vector<Vertex>& vertices) {
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);
		glGenBuffers(1, &vertex_buffer);
		glGenBuffers(1, &element_array);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		unbind();
	}

	void setElements(const std::vector<unsigned int>& elements) {
		bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * elements.size(), elements.data(), GL_STATIC_DRAW);
		unbind();
	}

	void bind() {
		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array);
	}
	void unbind() {
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void destroy() {
		unbind();
		glDeleteVertexArrays(1, &vertex_array);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteBuffers(1, &element_array);
		vertex_array = 0;
		vertex_buffer = 0;
		element_array = 0;
	}
};

const glm::vec3 offset{1.0f, 0.7f, 0.3f};
const auto& o = offset;

const std::vector<Vertex> quadVertices = 
	{   // pos(x,y,z)   , col(r,g,b)
		Vertex{o.x+ -1.0f,o.y+ -0.5f,o.z+ 0.0f, 1.f, 0.f, 0.f },
		Vertex{o.x+  1.0f,o.y+ -0.5f,o.z+ 0.0f, 0.f, 1.f, 0.f },
		Vertex{o.x+  1.0f,o.y+  0.5f,o.z+ 0.0f, 0.f, 0.f, 1.f },
		Vertex{o.x+  1.0f,o.y+  0.5f,o.z+ 0.0f, 0.f, 0.f, 1.f },
		Vertex{o.x+ -1.0f,o.y+  0.5f,o.z+ 0.0f, 0.f, 1.f, 0.f },
		Vertex{o.x+ -1.0f,o.y+ -0.5f,o.z+ 0.0f, 1.f, 0.f, 0.f }
	};

const std::vector<Vertex> bboxVertices = 
	{   // pos(x,y,z)   , col(r,g,b)
		Vertex{o.x+ -1.0f,o.y+ -0.7f,o.z+ -0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+  1.0f,o.y+ -0.7f,o.z+ -0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+  1.0f,o.y+  0.7f,o.z+ -0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+ -1.0f,o.y+  0.7f,o.z+ -0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+ -1.0f,o.y+ -0.7f,o.z+  0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+  1.0f,o.y+ -0.7f,o.z+  0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+  1.0f,o.y+  0.7f,o.z+  0.3f, 0.1f, 0.f, 0.8f },
		Vertex{o.x+ -1.0f,o.y+  0.7f,o.z+  0.3f, 0.1f, 0.f, 0.8f },
	};
const std::vector<unsigned int> bboxElements =
	{
		0, 1, 2, 3, 0, // bottom
		4, 5, 6, 7, 4, // top
		5, 1, 2, 6, 7, 3 // connect them
	};

static const char* vertex_shader_source = R"(
	#version 460

	layout(location = 0) in vec3 vCol;
	layout(location = 2) in vec3 vPos;

	uniform mat4 MVP;

	out smooth vec3 color;

	void main()
	{
		gl_Position = MVP * vec4(vPos, 1.0);
		color = vCol;
	}\
)";

static const char* fragment_shader_source = R"(
	#version 460

	in vec3 color;

	void main()
	{
		gl_FragColor = vec4(color, 1.0);
	}\
)";

static void glfw_error_callback(int error, const char* description)
{
	std::cout << "[GLFW Error] " << error << ": " << description << std::endl;
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void APIENTRY opengl_debug_message_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	/* Message Sources 
		Source enum                      Generated by
		GL_DEBUG_SOURCE_API              Calls to the OpenGL API
		GL_DEBUG_SOURCE_WINDOW_SYSTEM    Calls to a window - system API
		GL_DEBUG_SOURCE_SHADER_COMPILER  A compiler for a shading language
		GL_DEBUG_SOURCE_THIRD_PARTY      An application associated with OpenGL
		GL_DEBUG_SOURCE_APPLICATION      Generated by the user of this application
		GL_DEBUG_SOURCE_OTHER            Some source that isn't one of these
	*/
	/* Message Types
		Type enum                          Meaning
		GL_DEBUG_TYPE_ERROR                An error, typically from the API
		GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR  Some behavior marked deprecated has been used
		GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR   Something has invoked undefined behavior
		GL_DEBUG_TYPE_PORTABILITY          Some functionality the user relies upon is not portable
		GL_DEBUG_TYPE_PERFORMANCE          Code has triggered possible performance issues
		GL_DEBUG_TYPE_MARKER               Command stream annotation
		GL_DEBUG_TYPE_PUSH_GROUP           Group pushing
		GL_DEBUG_TYPE_POP_GROUP            foo
		GL_DEBUG_TYPE_OTHER                Some type that isn't one of these
	*/
	/* Message Severity
		Severity enum                    Meaning
		GL_DEBUG_SEVERITY_HIGH           All OpenGL Errors, shader compilation / linking errors, or highly - dangerous undefined behavior
		GL_DEBUG_SEVERITY_MEDIUM         Major performance warnings, shader compilation / linking warnings, or the use of deprecated functionality
		GL_DEBUG_SEVERITY_LOW            Redundant state change performance warning, or unimportant undefined behavior
		GL_DEBUG_SEVERITY_NOTIFICATION   Anything that isn't an error or performance issue.
	*/
	std::cout << "[OpenGL Error] " << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;
	GLuint vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	int currentWidth = 640;
	int currentHeight = 480;
	window = glfwCreateWindow(currentWidth, currentHeight, "Minimal OpenGL 4.6 Rendering", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(opengl_debug_message_callback, nullptr);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_CULL_FACE);

	std::cout << "Building shaders.\n" << std::endl;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		std::cout << "Error compiling or linking shaders. Abort." << std::endl;
		exit(EXIT_FAILURE);
	}

	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");

	const auto registerVertexAttributes = [&]() {
		glEnableVertexAttribArray(vpos_location);
		glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
		glEnableVertexAttribArray(vcol_location);
		glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 3));
	};

	RenderVertices quad;
	quad.init(quadVertices);
	quad.bind();
	registerVertexAttributes();
	quad.unbind();

	RenderVertices bbox;
	bbox.init(bboxVertices);
	bbox.bind();
	registerVertexAttributes();
	bbox.unbind();
	bbox.setElements(bboxElements);

	auto view = glm::lookAt(glm::vec3{0.0f, 0.0f, 3.0f}/*eye*/, glm::vec3{0.0f}/*center*/, glm::vec3{0.0f, 1.0f, 0.0f}/*up*/);
	float ratio = currentWidth/ (float) currentHeight;
	auto projection = glm::perspective(90.0f, ratio, 0.1f, 10.0f);

	while (!glfwWindowShouldClose(window))
	{
		{
			int width = currentWidth, height = currentHeight;
			glfwGetWindowSize(window, &width, &height);

			const bool hasNewWindowSize = (width != currentWidth) || (height != currentHeight);
			if (hasNewWindowSize) {
				currentWidth = width;
				currentHeight = height;
			}
		}

		ratio = currentWidth / (float) currentHeight;
		glViewport(0, 0, currentWidth, currentHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view = glm::lookAt(
			glm::vec3{2.0f, 0.0f, 2.0f},
			glm::vec3{0.0f},
			glm::vec3{0.0f, 1.0f, 0.0f});

		glm::mat4 model = glm::rotate(static_cast<float>(glfwGetTime() / 5.0f), glm::vec3{1.0f, 1.0f, 0.0f});

		const auto mvp = projection * view * model;

		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) glm::value_ptr(mvp));

		quad.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		quad.unbind();

		bbox.bind();
		glDrawElements(GL_LINE_STRIP, bboxElements.size(), GL_UNSIGNED_INT, (void*)0);
		bbox.unbind();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	quad.destroy();
	bbox.destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}