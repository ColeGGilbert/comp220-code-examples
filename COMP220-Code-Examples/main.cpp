#include <iostream>
#include <SDL.h>
#include <gl\glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL_opengl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

int xWidth[4] = { 640, 852, 1280, 1920 };
int yHeight[4] = { 360, 480, 720, 1080 };

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

struct Engine
{
	SDL_Window* window;
	SDL_GLContext glContext;
	GLuint programID;
};

struct Initialisation 
{
	Engine eng;
	int fail;
};

Initialisation InitialiseEngine(const char* windowName)
{
	Initialisation init;

	//Initialises the SDL Library, passing in SDL_INIT_VIDEO to only initialise the video subsystems
	//https://wiki.libsdl.org/SDL_Init
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		//Display an error message box
		//https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init failed", SDL_GetError(), NULL);
		init.fail = 1;
		return init;
	}

	//Create a window, note we have to free the pointer returned using the DestroyWindow Function
	//https://wiki.libsdl.org/SDL_CreateWindow
	init.eng.window =
		SDL_CreateWindow(
			windowName,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			xWidth[0], yHeight[0],
			SDL_WINDOW_OPENGL
		);

	//Checks to see if the window has been created, the pointer will have a value of some kind
	if (init.eng.window == nullptr)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateWindow failed", SDL_GetError(), NULL);
		//Close the SDL Library
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();
		init.fail = 1;
		return init;
	}

	init.eng.glContext = SDL_GL_CreateContext(init.eng.window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialise GLEW", (char*)glewGetErrorString(glewError), NULL);
	}

	init.eng.programID = LoadShaders("basicShaderTest.glsl", "basicShaderTest2.glsl");

	return init;
}

Engine i_engine;

void UpdateWindowSize(int i)
{
	SDL_SetWindowSize(i_engine.window, xWidth[i], yHeight[i]);
	glViewport(0, 0, xWidth[i], yHeight[i]);
	SDL_SetWindowPosition(i_engine.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

glm::mat4 ApplyTransformation(glm::mat4 object, float rotation = 0.0f, glm::vec3 rotatePos = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f))
{
	object = glm::rotate(object, rotation, rotatePos);
	object = glm::scale(object, scale);
	object = glm::translate(object, translation);
	return object;
}

int main(int argc, char ** argsv)
{
	srand(time(0));

	Initialisation init = InitialiseEngine("Cole's Random Colour Square");

	if (init.fail == 1) 
	{
		return 1;
	}

	i_engine = init.eng;

	#pragma region ObjectVertices
	// Creating Vertex Array ID
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		// Positions				// Colours
		-0.75f, -0.5f, 0.0f,		(float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100,   // Vertex B
		0.75f, -0.5f, 0.0f,			(float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100,   // Vertex C
		0.75f, 0.5f, 0.0f,			(float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100,   // Vertex D
		-0.75f, 0.5f, 0.0f,			(float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100, (float)(rand() % 100 + 1) / 100,   // Vertex A
	};

	// Triangle indices - note anticlockwise ordering!
	static const GLuint g_vertex_indices[] = {
		0, 1, 2,    // First Triangle, BCD
		2, 3, 0     // Second Triangle, DAB
	};

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6 * sizeof(GL_FLOAT),                  // stride
		(void*)0           // array buffer offset
	);

	// 2nd attribute buffer : colours
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6*sizeof(GL_FLOAT),                  // stride
		(void*)(3 * sizeof(GL_FLOAT))            // array buffer offset
	);

	// Element Buffer
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_vertex_indices), g_vertex_indices, GL_STATIC_DRAW);
	#pragma endregion

	// Transforms that are applied to the scene view object
	glm::mat4 model = glm::mat4(1.0f);
	model = ApplyTransformation(model, 90.0f, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f));

	glm::mat4 mvp, view, projection;
	glm::vec3 position(0, 0, -2), forward(0, 0, -1), right(1, 0, 0), rotation(0);
	const glm::vec4 cameraFace(0, 0, 1, 0);
	const float walkSpeed = 0.5f, rotSpeed = 0.1f;

	unsigned int transformLoc = glGetUniformLocation(i_engine.programID, "transform");

	bool fullscreen = false;

	int currentSize = 0;

	//Event loop, we will loop until running is set to false, usually if escape has been pressed or window is closed
	bool running = true;

	//SDL Event structure, this will be checked in the while loop
	SDL_Event ev;

	while (running)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);

		//Poll for the events which have happened in this frame
		//https://wiki.libsdl.org/SDL_PollEvent

		while (SDL_PollEvent(&ev))
		{
			//Switch case for every message we are intereted in
			switch (ev.type)
			{
				
				//QUIT Message, usually called when the window has been closed
			case SDL_QUIT:
				running = false;
				break;
				//KEYDOWN Message, called when a key has been pressed down

			case SDL_MOUSEMOTION:
			{
				rotation.y -= ev.motion.xrel * rotSpeed;
				rotation.x -= ev.motion.yrel * rotSpeed;

				glm::mat4 viewRotate(1.f);
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f));
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				forward = glm::normalize(glm::vec3(viewRotate * cameraFace));

				//right = glm::normalize(glm::vec3(viewRotate * cameraFace));
				
				break;
			}
			
			case SDL_KEYDOWN:
				//Check the actual key code of the key that has been pressed
				switch (ev.key.keysym.sym)
				{

				#pragma region ScreenResizing
				case SDLK_F11:
					fullscreen = !fullscreen;
					if (fullscreen) 
					{
						UpdateWindowSize(sizeof(xWidth) / sizeof(*xWidth)-1);
					}

					SDL_SetWindowFullscreen(i_engine.window, fullscreen);

					if (!fullscreen) 
					{
						UpdateWindowSize(currentSize);
					}
					break;

				case SDLK_UP:
					if (currentSize < sizeof(xWidth) / sizeof(*xWidth) - 1)
					{
						currentSize++;
						UpdateWindowSize(currentSize);
					}
					break;

				case SDLK_DOWN:
					if (currentSize > 0)
					{
						currentSize--;
						UpdateWindowSize(currentSize);
					}
					break;
				#pragma endregion

					//Escape key
				case SDLK_ESCAPE:
					running = false;
					break;

				#pragma region MovementControls
				case SDLK_w:
					position += walkSpeed * forward;
					break;

				case SDLK_s:
					position -= walkSpeed * forward;
					break;

				case SDLK_a:
					position += walkSpeed * right;
					break;

				case SDLK_d:
					position -= walkSpeed * right;
					break;
				}
				#pragma endregion
			}
		}

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(i_engine.programID);

		view = glm::lookAt(
			position,
			position + forward,
			glm::vec3(0, 1, 0)
		);

		projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
		//projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

		mvp = projection * view * model;

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		// Draw a triangle
		// glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

		// Draw a square (elements)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

		SDL_GL_SwapWindow(i_engine.window);
	}



	glDisableVertexAttribArray(0);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	SDL_GL_DeleteContext(i_engine.glContext);
	//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
	//https://wiki.libsdl.org/SDL_DestroyWindow
	SDL_DestroyWindow(i_engine.window);
	//https://wiki.libsdl.org/SDL_Quit
	SDL_Quit();

	return 0;
}