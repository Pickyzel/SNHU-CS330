//Tiffany McDonnell
//2-22-24
//Final Project
//CS-330

#include <iostream>    // allows all in iostream
#include <cstdlib>     // allows all to EXIT_FAILURE
#include <GL/glew.h>   // includes the GLEW library
#include <glfw3.h>     // includes the GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// includes different meshes
#include "meshes.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std; //access to standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "FinalProjectTiffanyMcDonnell"; // Lables window title

	const int WINDOW_WIDTH = 800;// defines window width
	const int WINDOW_HEIGHT = 800;// defines window height

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         //(acronym) vertex array object
		GLuint vbos[2];     //(acronym) vertix array buffer
		GLuint nVertices;   // Number of vertices of the mesh
		GLuint nIndices;    //number of vertices/indices
	};



	// Main GLFW window
	GLFWwindow* gWindow = nullptr;//creates main GLFW window

	// Shader program
	GLuint gProgramId;

	Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));//variables used for camera

	GLuint gTextureId0, gTextureId1, gTextureId2, gTextureId3, gTextureId4, gTextureId5, gTextureId6, gTextureId7;

	//Shape Meshes from Professor Brian
	Meshes meshes;

	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	float gDeltaTime = 0.0f;//variables used for time(frames)
	float gLastFrame = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//code for vertex shader program
const GLchar* vertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 vertexPosition; // VAP position 0 for vertex position data
layout(location = 1) in vec3 vertexNormal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexFragmentNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(vertexPosition, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexFragmentNormal = mat3(transpose(inverse(model))) * vertexNormal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);

///////////////////////////////////////////////////////////////////////////////////////////////////////


const GLchar* fragmentShaderSource = GLSL(440,

	in vec3 vertexFragmentNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec4 objectColor;
uniform vec3 ambientColor;
uniform vec3 light1Color;
uniform vec3 light1Position;
uniform vec3 light2Color;
uniform vec3 light2Position;
uniform vec3 light3Color;
uniform vec3 light3Position;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform bool ubHasTexture;
uniform float ambientStrength = 0.1f; // Set ambient or global lighting strength
uniform float specularIntensity1 = 100.8f;
uniform float highlightSize1 = 100.0f;
uniform float specularIntensity2 = 20.f;
uniform float highlightSize2 = 16.0f;
//uniform float specularIntensity3 = 100.8f;
//uniform float highlightSize3 = 100.0f;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting
	vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

	//**Calculate Diffuse lighting**
	vec3 norm = normalize(vertexFragmentNormal); // Normalize vectors to 1 unit
	vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color
	vec3 light2Direction = normalize(light2Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color

	//**Calculate Specular lighting**
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
	vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;
	vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;

	//**Calculate phong result**
	//Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate);
	vec3 phong1;
	vec3 phong2;

	if (ubHasTexture == true)
	{
		phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
	}
	else
	{
		phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
	}

	fragmentColor = vec4(phong1 + phong2, 1.0); // Send lighting results to GPU
	//fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Functions
bool UInitialize(int, char* [], GLFWwindow** window);//Initialize the window
bool inPerspective = true;
void UResizeWindow(GLFWwindow* window, int width, int height);//Resizes the window to new width and new height
void UProcessInput(GLFWwindow* window);//renders graphics
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);//function for mouse position
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);//function for mouse scroll
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);//fuction for mouse button
void URender();//renders different shapes
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);//creates shader program
void UDestroyShaderProgram(GLuint programId);//releases data related to programId
bool UCreateTexture(const char* filename, GLuint& textureId);//creates textures
void UDestroyTexture(GLuint textureId);//releases data related to textureId


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])//Main function
{


	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	meshes.CreateMeshes();//Calls VBO

	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))//creates shader 
		return EXIT_FAILURE;

	//following if statement load in all textures and assign to a variable
	const char* texFilename = "TopColorSolid.png";//color provided through picture of top by Tiffany
	if (!UCreateTexture(texFilename, gTextureId2))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	texFilename = "CandleLid.png";//color provided through picture taken by Tiffany of a Mainstay candle.
	if (!UCreateTexture(texFilename, gTextureId0))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	texFilename = "Floor.png";//texure provided by photo of floor by Tiffany
	if (!UCreateTexture(texFilename, gTextureId1))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	texFilename = "BoxPrint.png";//Photo taken by Tiffany Print designed by hallmark
	if (!UCreateTexture(texFilename, gTextureId3))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "Eye.png";//pictue taken by Tiffany
	if (!UCreateTexture(texFilename, gTextureId4))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "TopColor.png";//picture taken by Tiffany
	if (!UCreateTexture(texFilename, gTextureId5))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	texFilename = "pie6.png";//picture taken by Tiffany // rights go t mainstay candle
	if (!UCreateTexture(texFilename, gTextureId6))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId0);//binds TopColorSolid
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTextureId1);//binds CandleLic
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gTextureId2);//binds Floor
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gTextureId3);//binds Box Print
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gTextureId4);//binds Eye
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gTextureId5);//binds Top Color
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gTextureId6);//binds pie6



	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//Sets the background color of the window to black
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//function used to see outline of pyramid

	gCamera.Position = glm::vec3(0.0f, 1.0f, 16.0f);
	gCamera.Front = glm::vec3(0.0, 0.0, -1.0f);
	gCamera.Up = glm::vec3(0.0, 1.0, 0.0);

	while (!glfwWindowShouldClose(gWindow))//reder loop  so window doesn't instantly close but will close if window should close is true
	{
		float currentFrame = glfwGetTime();//preframe timing
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		UProcessInput(gWindow);//input
		URender();//Renders this frame
		glfwPollEvents();
	}

	meshes.DestroyMeshes();//releases the data associated with meshes

	//releases data to textures
	UDestroyTexture(gTextureId2);
	UDestroyTexture(gTextureId0);
	UDestroyTexture(gTextureId1);
	UDestroyTexture(gTextureId3);
	UDestroyTexture(gTextureId4);
	UDestroyTexture(gTextureId5);
	UDestroyTexture(gTextureId6);
	UDestroyShaderProgram(gProgramId);//releases the data associated with gProgramId

	exit(EXIT_SUCCESS); //will terminate the program
}

bool UInitialize(int argc, char* argv[], GLFWwindow** window)//Inialize GLFW,GLEW and creates the window
{
	// GLFW: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//window creation
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		//if window failed to create will print message and terminate
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return false;
	}

	//sets window to current and resize
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);

	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //tells window to capture the mouse

	//Initalizes Glew
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		cerr << glewGetErrorString(GlewInitResult) << endl;
		return false;
	}

	return true;
}

void UProcessInput(GLFWwindow* window)//Processes all the input
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) //W moves camera Forward
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) //S moves camera Backward
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)  //A moves camera Left
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) //D moves camera Right
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)//Q moves camera upward
		gCamera.ProcessKeyboard(UP, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)//E moves camera downward
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)//P switchs between ortho and perspective view
	{

		inPerspective = true;
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)//O switchs between ortho and perspective view
	{

		inPerspective = false;
	}


}

/////////////////////////////////////////////////////////////////////////////////////

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)//fuction is called when ever mouse is moved
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos;

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}

void UResizeWindow(GLFWwindow* window, int width, int height)//will call whenever window size is changed
{
	glViewport(0, 0, width, height);
}


void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)//function called whenever mouse scroll scrolls
{
	gCamera.ProcessMouseScroll(yoffset);//changes movement speed
}

void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)//used to handle mouse button events
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT://case wheen left mouse button is clicked
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE://case when middle mouse button is clicked
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT://case when right mouse button is clicked
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;//unhandled event for the mouse
		break;
	}
}

void URender()//calls the render frame
{
	//mesh varibles to allow tranformation
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint viewPosLoc;
	GLint ambStrLoc;
	GLint ambColLoc;
	GLint light1ColLoc;
	GLint light1PosLoc;
	GLint light2ColLoc;
	GLint light2PosLoc;
	GLint light3ColLoc;
	GLint light3PosLoc;
	GLint objectColorLoc;
	GLint specInt1Loc;
	GLint highlghtSz1Loc;
	GLint specInt2Loc;
	GLint highlghtSz2Loc;
	GLint specInt3Loc;
	GLint highlghtSz3Loc;
	GLint uHasTextureLoc;
	bool ubHasTextureVal;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 rotation2;
	glm::mat4 rotation3;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;


	glEnable(GL_DEPTH_TEST);//allows z-depth

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//clears the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transforms the camera
	//view = gCamera.GetViewMatrix();

	//If statement that handles when P button is press to switch between perspective and ortho view
	if (inPerspective)
	{
		view = gCamera.GetViewMatrix();
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);//creates prespective view
	}
	else// in ortho
	{
		view = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));
		projection = glm::ortho(-8.0f, 8.0f, -7.0f, 7.0f, 0.1f, 100.0f);
	}

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	viewPosLoc = glGetUniformLocation(gProgramId, "viewPosition");
	ambStrLoc = glGetUniformLocation(gProgramId, "ambientStrength");
	ambColLoc = glGetUniformLocation(gProgramId, "ambientColor");
	light1ColLoc = glGetUniformLocation(gProgramId, "light1Color");
	light1PosLoc = glGetUniformLocation(gProgramId, "light1Position");
	light2ColLoc = glGetUniformLocation(gProgramId, "light2Color");
	light2PosLoc = glGetUniformLocation(gProgramId, "light2Position");
	light3ColLoc = glGetUniformLocation(gProgramId, "light3Color");
	light3PosLoc = glGetUniformLocation(gProgramId, "light3Position");
	objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
	specInt1Loc = glGetUniformLocation(gProgramId, "specularIntensity1");
	highlghtSz1Loc = glGetUniformLocation(gProgramId, "highlightSize1");
	specInt2Loc = glGetUniformLocation(gProgramId, "specularIntensity2");
	highlghtSz2Loc = glGetUniformLocation(gProgramId, "highlightSize2");
	specInt3Loc = glGetUniformLocation(gProgramId, "specularIntensity2");
	highlghtSz3Loc = glGetUniformLocation(gProgramId, "highlightSize2");
	uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the camera view location
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);
	//set ambient lighting strength
	glUniform1f(ambStrLoc, 0.4f);
	//set ambient color
	glUniform3f(ambColLoc, 0.2f, 0.2f, 0.2f);

	//glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);
	//glUniform3f(light1PosLoc, 2.0f, 5.0f, 5.0f);
	//glUniform3f(light2ColLoc, 0.1f, 0.1f, 0.1f);
	//glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);
	//set specular intensity
	//glUniform1f(specInt1Loc, 1.0f);
	//glUniform1f(specInt2Loc, 0.1f);
	//set specular highlight size
	//glUniform1f(highlghtSz1Loc, 12.0f);
	//glUniform1f(highlghtSz2Loc, 12.0f);

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	///***********************************************************************************************Plane************************************************
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);


	scale = glm::scale(glm::vec3(6.0f, 1.0f, 6.0f));//scales the plane

	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));//rotates the plane

	translation = glm::translate(glm::vec3(0.0f, -3.8f, -12.0f));//postions the plane

	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);

	///// set diffuse lighting for PLANE OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, 2.0f, 5.0f, 5.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 2.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for PLANE OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .9f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 12.0f);                //specular light highlight (high for specular)
	/////-----------------------------------------------------------

	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 1);

	glDrawArrays(GL_TRIANGLES, 0, meshes.gPlaneMesh.nIndices);//Draws indices

	glBindVertexArray(meshes.gPlaneMesh.vao);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);



	///**************************************************************Cone on Top***************************************************************
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gConeMesh.vao);

	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));//scales the cone

	rotation = glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));//rotates cone on x axis
	rotation3 = glm::rotate(glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotates cone on y axis
	rotation2 = glm::rotate(glm::radians(-44.0f), glm::vec3(0.0f, 0.0f, 1.0f));//rotates cone on z axis
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -3.00f, -8.0f));// Position the object
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * rotation2 * rotation3 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 1.0f, 1.0f);

	/*lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
	lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
	lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	lightingShader.setFloat("material.shininess", 32.0f);*/

	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);

	///// set diffuse lighting for Cone OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, -3.0f, 3.0f, -20.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .7f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 100.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for cone OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .9f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 12.0f);                //specular light highlight (high for specular)

	///// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light3PosLoc, 2.0f, 5.0f, 20.0f);    //diffuse light position
	glUniform1f(specInt3Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz3Loc, 20.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom

	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 2);
	glDrawArrays(GL_TRIANGLE_STRIP, 36, 108);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	///**************************************************************************CYLINDER(Handle of Top)*********************************
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	scale = glm::scale(glm::vec3(0.21f, 0.7f, .20f));//scales the cylinder

	rotation = glm::rotate(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));//rotates cylinder on x axis
	rotation2 = glm::rotate(glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));//rotates cylinder on z axis
	rotation3 = glm::rotate(glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotates cylinder on y axis

	translation = glm::translate(glm::vec3(1.60f, -3.10f, -8.0f));//Position the object
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * rotation2 * rotation3 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 2);

	///// set diffuse lighting for Top Handle OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, -3.0f, 3.0f, -20.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .7f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 100.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for Top Handle OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .9f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 12.0f);                //specular light highlight (high for specular)

	/// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light3PosLoc, 2.0f, 5.0f, 20.0f);    //diffuse light position
	glUniform1f(specInt3Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz3Loc, 20.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	glBindVertexArray(0);//deactivates the vao


	///*************************************************************************Sphere for ball**********************************************************
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gSphereMesh.vao);

	scale = glm::scale(glm::vec3(0.75f, 0.75f, 0.75f));//scales the sphere

	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));//rotates the sphere

	translation = glm::translate(glm::vec3(-2.4f, -3.03f, -7.3f));//Positions the sphere

	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 4);

	///// set diffuse lighting for BALL OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, -3.0f, 3.0f, -20.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .7f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 100.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for BALL OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 3.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .9f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 12.0f);                //specular light highlight (high for specular)
	/////-----------------------------------------------------------

		///// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light3PosLoc, 2.0f, 5.0f, 20.0f);    //diffuse light position
	glUniform1f(specInt3Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz3Loc, 20.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	///******************************************************************************************Box******************************************************
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	scale = glm::scale(glm::vec3(2.5f, 2.75f, 1.0f));//scales the box

	rotation = glm::rotate(-3.0f, glm::vec3(0.0, 1.0f, 0.0f));//rotates the box

	translation = glm::translate(glm::vec3(-3.5f, -2.41f, -12.0f));//positions the box

	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.0f, 1.0f);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 3);

	///// set diffuse lighting for Box OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, -3.0f, 3.0f, -20.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .7f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 100.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for Box OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .7f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 11.0f);                //specular light highlight (high for specular)

	/// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light3PosLoc, 2.0f, 5.0f, -20.0f);    //diffuse light position
	glUniform1f(specInt3Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz3Loc, 20.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	///****************************************************************************************Cylinder(Bottom of Candle)***************************************
// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	scale = glm::scale(glm::vec3(2.0f, 2.8f, 2.0f));//scales the bottom of candle

	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));//rotates the bottom of candle

	translation = glm::translate(glm::vec3(2.5f, -3.7f, -13.5f));//positions the bottom of candle

	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 6);

	///// set diffuse lighting for Candle Bottom OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, 2.0f, 5.0f, 5.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .3f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 64.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for Candle Bottom OBJECT (light source 2)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, 2.0f, 5.0f, 5.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .3f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 64.0f);                //specular light highlight (high for specular)

	/////// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	//glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	//glUniform3f(light3PosLoc, 2.0f, 3.0f, -20.0f);    //diffuse light position
	//glUniform1f(specInt3Loc, 0.5f);                    //diffuse light intensity (low for diffuse)
	//glUniform1f(highlghtSz3Loc, 64.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	////****************************************************************************************Cylinder(Top of Candle)********************************************** 
// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	scale = glm::scale(glm::vec3(2.2f, 0.55f, 2.2f));//scales the top of candle

	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));//rotates the top of candle

	translation = glm::translate(glm::vec3(2.5f, -1.0f, -13.5f));//positions the top of candle

	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

	///// set diffuse lighting for Top of Candle OBJECT (light source 1)---
	glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light1PosLoc, 2.0f, 5.0f, 5.0f);    //diffuse light position
	glUniform1f(specInt1Loc, .3f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz1Loc, 2.0f);                //diffuse light highlight (low for diffuse)
	/////---------------------------------------------------------

	///// set specular lighting for Top of Candle OBJECT (light source 2)---
	glUniform3f(light2ColLoc, 0.9f, 0.8f, 0.2f);    //specular light color
	glUniform3f(light2PosLoc, -2.0f, 5.0f, 5.0f);    //specular light position
	glUniform1f(specInt2Loc, .5f);                    //specular light intensity (high for specular)
	glUniform1f(highlghtSz2Loc, 10.0f);                //specular light highlight (high for specular)

	///// set diffuse lighting for Candle Bottom OBJECT (light source 3)---
	glUniform3f(light3ColLoc, 1.0f, 1.0f, 1.0f);    //diffuse light color     
	glUniform3f(light3PosLoc, 2.0f, 5.0f, 20.0f);    //diffuse light position
	glUniform1f(specInt3Loc, .5f);                    //diffuse light intensity (low for diffuse)
	glUniform1f(highlghtSz3Loc, 20.0f);                //diffuse light highlight (low for diffuse)

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	glfwSwapBuffers(gWindow);   //swaps buffers for pressed and released events and refreshes each frame
}

void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbos);
}

// UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	programId = glCreateProgram();//shader program object

	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);//creates the vertex and fragment objects
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	glCompileShader(vertexShaderId); //compiles the shaders to the program
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);

	//error handling if vertexshader compilation fails
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader

	//error handling if fragmentshader compileation fails
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glAttachShader(programId, vertexShaderId);//attached compiled shaders to the program
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   //links shader to the program

	//error handling if shaders to no link to program
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    //uses program

	return true;
}


void UDestroyShaderProgram(GLuint programId)//released data
{
	glDeleteProgram(programId);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{

		//generates texture
		glGenTextures(1, &textureId);
		//binds textureId to GL_TEXTURE_2D
		glBindTexture(GL_TEXTURE_2D, textureId);

		//Sets texture wrapping to repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		//Generates the Mipmap
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 2);

		return true;
	}


	return false;
}


//release date to textureId
void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}