#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Windows.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 lightRotation;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 cloudModel = glm::mat4(1.0f);

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLuint cloudModelLoc;

int mode = 0;
double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
bool presentation = false;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.05f;
GLfloat lightAngle;
float cloudSpeed = 0.0f;
float cloudStep = 0.5f;
float xCam = 0.0f;
float yCam = 0.0f;
float zCam = 3.0f;
float timeCam = 0.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D myScene;
gps::Model3D cloud;
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader mySkyBoxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    // Update the camera's yaw and pitch based on the cursor offset
    cameraYaw += xoffset * 0.1f;
    cameraPitch += yoffset * 0.1f;

    // Make sure the camera's pitch stays within a certain range
    if (cameraPitch > 89.0f)
        cameraPitch = 89.0f;
    if (cameraPitch < -89.0f)
        cameraPitch = -89.0f;

    myCamera.rotate(cameraPitch, cameraYaw);
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_P]) {
        mode += 1;
        if(mode % 3 == 0)
             glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if(mode % 3 == 1)
             glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   
        if(mode % 3 == 2)
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        Sleep(400);
    }

    if (pressedKeys[GLFW_KEY_T]) {
        gps::Camera myCamera2(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, -10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        if (presentation == false) {
            presentation = true;
            myCamera = myCamera2;
        }
        else {
            presentation = false;
            myCamera = myCamera2;
            timeCam = 0.0f;
        }
    }

    if (presentation == true) {
        if(timeCam < 15.0f)
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        else if(timeCam < 30.0f)
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        else if(timeCam < 50.0f)
            myCamera.move(gps::MOVE_LEFT, cameraSpeed);

        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        timeCam += 0.05f;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 992, "OpenGL Project Core");
   // myWindow.Create(800, 600, "OpenGL Project Core");

}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    myScene.LoadModel("models/scene/untitled.obj");
    cloud.LoadModel("models/cloud/untitled.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
    mySkyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
    mySkyBoxShader.useShaderProgram();
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    //model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,  0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	//glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 1000.0f;
    glm::mat4 lightProjection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {

    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw scene
    myScene.Draw(shader);

    cloudSpeed += cloudStep;
    if (cloudSpeed > 50.0f || cloudSpeed < 0.0f){
        cloudStep = -cloudStep;
    }
    cloudModel = glm::translate(glm::mat4(1.0f), glm::vec3(cloudSpeed, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cloudModel));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    cloud.Draw(shader);

    //draw skybox
    mySkyBoxShader.useShaderProgram();
    mySkyBox.Draw(mySkyBoxShader, view, projection);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // depth maps creation pass
    //TODO - Send the light-space transformation matrix to the depth map creation shader and
    //		 render the scene in the depth map
    //render the scene to the depth buffer
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // render depth map on screen - toggled with the M key
    if (showDepthMap) {
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT);

        //screenQuadShader.useShaderProgram();
        depthMapShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(depthMapShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        myScene.Draw(depthMapShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {

        // final scene rendering pass (with shadows)

        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myBasicShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));

        drawObjects(myBasicShader, false);
    }

}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //cleanup code for your own data
    //glfwTerminate();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    
    //SkyBox
    std::vector<const GLchar*> faces;
    faces.push_back("textures/skybox/hills_rt.tga");
    faces.push_back("textures/skybox/hills_lf.tga");
    faces.push_back("textures/skybox/hills_up.tga");
    faces.push_back("textures/skybox/hills_dn.tga");
    faces.push_back("textures/skybox/hills_bk.tga");
    faces.push_back("textures/skybox/hills_ft.tga");
    mySkyBox.Load(faces);

	initModels();
	initShaders();
	initUniforms();
    initFBO();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
