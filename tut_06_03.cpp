#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
//#include <GL/glut.h> //new for wire shpere
//#include <GL/GLUT/glut.h> //new for wire shpere
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
const char* const WINDOW_TITLE = "Tutorial 6.3"; // Macro for window title

// Variables for window width and height
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Stores the GL data relative to a given mesh
struct GLMesh
{
    GLuint vao;         // Handle for the vertex array object
    GLuint vbo;         // Handle for the vertex buffer object
    GLuint nVertices;    // Number of indices of the mesh
};

// Main GLFW window
GLFWwindow* gWindow = nullptr;
// Triangle mesh data
GLMesh gMesh;
// Texture
GLuint gTextureId;
GLuint gTextureIdTolberone;      // NEW
GLuint gTextureIdTolberoneFront;
GLuint gTextureIdTolberoneBack;
GLuint gTextureIdTolberoneSmallSide1;
GLuint gTextureIdCubeSide;
GLuint gTextureIdCubeTop;
GLuint gTextureIdBall;

glm::vec2 gUVScale(1.0f, 1.0f);
GLint gTexWrapMode = GL_REPEAT;

// Shader programs
GLuint gCubeProgramId;
GLuint gLampProgramId;

// camera
Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
float gLastX = WINDOW_WIDTH / 2.0f;
float gLastY = WINDOW_HEIGHT / 2.0f;
bool gFirstMouse = true;

// timing
float gDeltaTime = 0.0f; // time between current frame and last frame
float gLastFrame = 0.0f;

// Subject position and scale
glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
glm::vec3 gCubeScale(1.0f);

// Cube and light color
//m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
glm::vec3 gLightColor(0.8f, 1.0f, 0.8f);

// Light position and scale
glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
glm::vec3 gLightScale(0.3f);

// NEW*******************
//new Code for second light color scale and position
glm::vec3 gLightPosition2(0.5f, 1.5f, 3.0f);
glm::vec3 gLightScale2(0.3f);
glm::vec3 gLightColor2(1.0f, 1.0f, 1.0f);
glm::vec3 gObjectColor2(0.f, 1.2f, 0.0f);

// Lamp animation
bool gIsLampOrbiting = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char*[], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh &mesh);
void UDestroyMesh(GLMesh &mesh);
bool UCreateTexture(const char* filename, GLuint &textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId);
void UDestroyShaderProgram(GLuint programId);
bool P_KeyPress = false;


/* Cube Vertex Shader Source Code*/
const GLchar * cubeVertexShaderSource = GLSL(440,

    layout (location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout (location = 1) in vec3 normal; // VAP position 1 for normals
    layout (location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Cube Fragment Shader Source Code*/
const GLchar * cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor; // For outgoing cube color to the GPU

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

    // NEW********************
    uniform vec3 lightPos2;
    uniform vec3 lightColor2;

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
        float ambientStrength = 0.5f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * lightColor; // Generate diffuse light color

        //Calculate Specular lighting*/
        float specularIntensity = 0.8f; // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate phong result
        vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }
);


/* Lamp Shader Source Code*/
const GLchar * lampVertexShaderSource = GLSL(440,

    layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


/* Fragment Shader Source Code*/
const GLchar * lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char *image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    //New v.1
    float cameraSpeed = 2.5f;
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../../resources/textures/plane2.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/tolberone.png";
    if (!UCreateTexture(texFilename, gTextureIdTolberone))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/tolberoneFront.png";
    if (!UCreateTexture(texFilename, gTextureIdTolberoneFront))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/tolberoneBack.png";
    if (!UCreateTexture(texFilename, gTextureIdTolberoneBack))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/tolberoneSmallSide1.png";
    if (!UCreateTexture(texFilename, gTextureIdTolberoneBack))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "../../resources/textures/cubeSide.png";
    if (!UCreateTexture(texFilename, gTextureIdCubeSide))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/cubeTop.png";
    if (!UCreateTexture(texFilename, gTextureIdCubeTop))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/ball.png";
    if (!UCreateTexture(texFilename, gTextureIdBall))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId);

    // Release shader programs
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    // UP Q
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    // DOWN E
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    // ortho and perspective
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        P_KeyPress = !P_KeyPress; // this toggles the global P_KeyPress and activates or deactivates glm::mat4 projection = glm::ortho(-7.0f, 7.0f, -5.0f, 5.0f, 0.1f, 100.0f);
        //gCamera.ProcessKeyboard(SWITCH, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = {1.0f, 0.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            if (action == GLFW_PRESS)
                cout << "Left mouse button pressed" << endl;
            else
                cout << "Left mouse button released" << endl;
        }
        break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
        {
            if (action == GLFW_PRESS)
                cout << "Middle mouse button pressed" << endl;
            else
                cout << "Middle mouse button released" << endl;
        }
        break;

        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            if (action == GLFW_PRESS)
                cout << "Right mouse button pressed" << endl;
            else
                cout << "Right mouse button released" << endl;
        }
        break;

        default:
            cout << "Unhandled mouse button event" << endl;
            break;
    }
}



// Functioned called to render a frame
void URender()
{
  /*  
  void gluSphere(GLUquadric * quad, GLdouble radius, GLint slices, GLint stacks);
  // quad is the new object created with gluNewQuadric.
  // GLU primative declared with GLUquadric *gluNewQu
  GLUquadric* gluNewQuadric(void)
  // Use a 10 slices and  10 stacks

  //try simple sphere
    glColor3f(1, 0, 0);
    GLUquadric* quad;
    quad = gluNewQuadric();
    gluSphere(quad, 25, 100, 20);
    glTranslatef(2, 2, 2);*/

    // Lamp orbits around the origin
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);
    


    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gMesh.vao);

    // CUBE: draw cube
    //----------------
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

    //  ***********THIS IS FOR CAMERA VIEW************
// camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // **********TOGGLES ORTHO AND PROJECTION with use of global variable bool P_KeyPress   
    glm::mat4 projection;
    if (P_KeyPress == true)
        projection = glm::ortho(-7.0f, 7.0f, -5.0f, 5.0f, 0.1f, 100.0f); //left, right , bottom, top, zNear, zFar  // this moves the camera left right up down ect..
    else projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
     
    //new sphere fixme 

    //shaneQuad = gluNewQuadric();
    //slices = stacks = resolution;
    //slices = 10;
    //stacks = 10;

    //gluSphere(shaneQuad, radius, slices, stacks);

    //gluQuadricTexture(quad,1);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


        // bind textures on corresponding texture units
    // ------------------------------PLane--------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, 6); /// start of array,  count

    // -------------------------------------Tolberone base---------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTolberone);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    //// -------------------------------------Tolberone Front---------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTolberoneBack);
    glDrawArrays(GL_TRIANGLES, 11, 6);

    //////// -------------------------------------Tolberone rear---------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTolberoneFront);
    glDrawArrays(GL_TRIANGLES, 17, 6);
    //////// -------------------------------------Tolberone Side-------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTolberoneSmallSide1);
    glDrawArrays(GL_TRIANGLES, 23, 3);
    //////// -------------------------------------Tolberone Side-------------------------------------------

 
    /// -------------------------------------Cube Base-------------------------------
 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeTop);
    glDrawArrays(GL_TRIANGLES, 30, 6);

   
    /// -------------------------------------Cube right Side-------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeSide);
    glDrawArrays(GL_TRIANGLES, 36, 6);

       //cube front
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeSide);
    glDrawArrays(GL_TRIANGLES, 42, 6);

    //cube Left side
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeSide);
    glDrawArrays(GL_TRIANGLES, 48, 6);

        //cube back side
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeSide);
    glDrawArrays(GL_TRIANGLES, 54, 6);

    //cube top
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeTop);
    glDrawArrays(GL_TRIANGLES, 60, 6);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBall);
    glDrawArrays(GL_TRIANGLES, 66, 12);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBall);
    glDrawArrays(GL_TRIANGLES, 78, 13);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBall);
    glDrawArrays(GL_TRIANGLES, 91, 14);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBall);
    glDrawArrays(GL_TRIANGLES, 105, 12);


    // LAMP: draw lamp*******************************************************************
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, 6);


    // LAMP2: draw lamp*******************************************************************
    //----------------
    glUseProgram(gLampProgramId);
    glm::vec3 gLightColor;
    gLightColor = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 glightColor(0.0f, 1.0f, 0.0f);
    //LightColor = glm::vec3(0.0f, 1.0f, 0.0f);
    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition2) * glm::scale(gLightScale2);
    //uniform vec3 gLightColor2;
            //Calculate Ambient lighting*/
    //float ambientStrength = 0.1f; // Set ambient or global lighting strength
    ////ambient = ambientStrength * lightColor; // Generate ambient light color
    //GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    //GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    //glUniform3f(objectColorLoc, gObjectColor2.r, gObjectColor2.g, gObjectColor2.b);
   // glUniform3f(lightColorLoc, gLightColor2.r, gLightColor2.g, gLightColor2.b);

    //lightColor = ;
   // glm::vec3 gl (0.f, 1.2f, 0.0f);
    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    //************NEW WIRE SPHERE

    //glutWireSphere(30, 10, 10);
    //model = glm::translate(2.0f, 0.0f, 0.0f) * glm::scale(0.0f, 0.0f, 0.0f);
    //glTranslatef(0.5, 0.5, 0.5);
    //GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    //GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    //GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    //GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    //GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    //GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    //GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    //// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    //glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    //glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    //glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    //glutWireSphere(30, 10, 10);
    //glColor3f(1.0, 0.0f, 0.0f);

    //    //NEW for the sphere
    /*GLUquadric* shaneQuad;
    GLdouble radius = 0.2;
    GLint slices, stacks;
    GLint nsides, rings;
    gluSphere(shaneQuad, 1, 10, 10);*/
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh &mesh)
{
     // Position and Color data
    GLfloat verts[] = {

        //************PLANE**********************
        // 0-5  mapping arrays 
        
        1.0f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    1.0f,  1.0f, //top right
        1.0f, -1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    0.0f,  1.0f, //bottom right
       -1.0f, -1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    0.0f,  0.0f, // bottom left

       -1.0f, -1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    0.0f,  0.0f, // bottom left,  BL of tex
       -1.0f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    1.0f,  0.0f, // top left,     TL of tex
        1.0f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,    1.0f,  1.0f, //top right,  

       // 6-11    //***PYRAMID BASE 2 TRIANGLES
        //6           //Negative Z Normal  Texture Coords.
        0.25f, -0.5f,  0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  1.0f, // 0 Top Right  This is the vertex corrdinate Zero for pyramid then clockwise around the base 
        0.15f, -0.65f, 0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f,// 1  Bottom Right  
       -0.75f, -0.15f, 0.0001f,  0.0f,  0.0f,  1.0f,   0.0f,  0.0f, // 2 Bottom Left 

       -0.75f, -0.15f, 0.0001f,  0.0f,  0.0f,  1.0f,   0.0f,  0.0f, // 2 Bottom Left
       -0.7f,   0.0f,  0.0001f,  0.0f,  0.0f,  1.0f,   0.0f,  1.0f, // 3 Top Left 
        0.25f, -0.5f,  0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  1.0f, // 0 Top Right
        //11
                 //*****FRONT SIDE OF PYRAMID********
         //12-17
        0.2f,  -0.575f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, //  CAP RIGHT of pyramid
        0.15f, -0.65f, 0.0001f,  0.0f,  0.0f,  1.0f,   0.0f,  1.0f,// 1  Bottom Right
       -0.75f, -0.15f, 0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 2 Bottom Left

       -0.75f, -0.15f, 0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 2 Bottom Left
       -0.725f,-0.075f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // CAP LEFT of pyramid
        0.2f,  -0.575f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, //  CAP RIGHT of pyramid
        //*****REAR SIDE OF PYRAMID********
       //18-23
        0.25f, -0.5f,  0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  1.0f, // 0 Top Right
        0.2f,  -0.575f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, //  CAP RIGHT of pyramid
       -0.7f,   0.0f,  0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 3 Top Left of base

       -0.725f,-0.075f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // CAP LEFT of pyramid
       -0.7f,   0.0f,  0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 3 Top Left of base
        0.2f,  -0.575f, 0.2f,    0.0f,  0.0f,  1.0f,   1.0f,  0.0f, //  CAP RIGHT of pyramid

            //*****RIGHT SIDE OF PYRAMID********
    // 23
        0.25f, -0.5f,  0.0001f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, // 0 Top Right 
        0.2f,  -0.575f, 0.2f,    0.0f,  0.0f,  1.0f,  1.0f,  0.0f, //  CAP RIGHT of pyramid
        0.15f, -0.65f, 0.0001f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,// 1  Bottom Right

     //*****LEFT SIDE OF PYRAMID********
    //26
     -0.7f,    0.0f,    0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 3 Top Left of base
     -0.725f, -0.075f,  0.2f,     0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // CAP LEFT of pyramid
     -0.75f,  -0.15f,   0.0001f,  0.0f,  0.0f,  1.0f,   1.0f,  0.0f, // 2 Bottom Left


    // ********CUBE***********
    //Base
    //30
     -0.22f, 0.75f, 0.00001f, 0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // 0
     -0.35f, 0.25f, 0.00001f, 0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // 1
     -0.7f,  0.86f, 0.00001f, 0.0f,  0.0f,  1.0f,   0.0f, 1.0f,// 3

     -0.7f,  0.86f, 0.00001f, 0.0f,  0.0f,  1.0f,   0.0f, 1.0f,// 3
     -0.35f, 0.25f, 0.00001f, 0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // 1
     -0.86f, 0.38f, 0.00001f, 0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // 2

     //right side
     //36
    -0.22f, 0.75f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 0 
    -0.35f, 0.25f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1
    -0.22f, 0.75f, 1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // 4

    -0.22f, 0.75f, 1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // 4
    -0.35f, 0.25f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1
    -0.35f, 0.25f, 1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 5 

    //front side
    //42
    -0.35f, 0.25f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1   
    -0.86f, 0.38f, 0.00001f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 2   
    -0.35f, 0.25f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 5

    -0.86f, 0.38f, 0.00001f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 2   
    -0.35f, 0.25f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 5
    -0.86f, 0.38f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f,// 6

    // left side
    //48
    -0.86f, 0.38f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 2
    -0.7f,  0.86f, 0.00001f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,// 3
    -0.86f, 0.38f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f,// 6

    -0.86f, 0.38f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f,// 6
    -0.7f,  0.86f, 1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 0.0f,// 7
    -0.7f,  0.86f, 0.00001f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,// 3
    //back side
    //54
    -0.7f,  0.86f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,// 3
    -0.22f, 0.75f, 0.00001f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 0
    -0.22f, 0.75f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 4

    -0.22f, 0.75f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 4
    -0.7f,  0.86f, 1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 1.0f,// 7
    -0.7f,  0.86f, 0.00001f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,// 3
    // top
    //60
    -0.22f, 0.75f, 1.0f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // 4  
    -0.35f, 0.25f, 1.0f,      0.0f, 0.0f, 1.0f,    1.0f, 0.0f, // 5 
    -0.86f, 0.38f, 1.0f,      0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// 6

    -0.86f, 0.38f, 1.0f,      0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// 6
    -0.22f, 0.75f, 1.0f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // 4
    -0.7f,  0.86f, 1.0f,      0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// 7
 // sphere
     
           0.41f, -0.02f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //B
           0.977f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.7f, 0.0f, //I
           0.76f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.2f, //F

           0.41f, -0.02f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //B
           0.76f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.2f, //F
           0.06f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.8f, //H

           0.41f, -0.02f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //B
           -0.157f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, //K
           0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, //A

           0.41f, -0.02f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //B
           0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, //A
           0.977f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.7f, 0.0f, //I

            0.76f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //F
            0.977f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //I
            0.977f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.5f, 0.66f, //J

            0.76f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //F
            0.41f, -0.02f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //D
            0.977f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.5f, 0.66f, //J

           0.76f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //F
           0.06f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //H
            0.41f, -0.02f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //D

            0.06f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //H
            0.41f, -0.02f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //D
            -0.157f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, //L

         0.06f, -0.237f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //H
         -0.157f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, //K
         -0.157f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, //L

         -0.157f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, //K
         -0.157f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, //L
         0.06f,  0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //G

         -0.157f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, //K
         0.06f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //G
         0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //A

         0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //A
         0.06f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.5f, 0.66f, //G
         0.76f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //E

         0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.0f, 0.59f, //A
         0.76f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.5f, 0.33f, //E
         0.977f, 0.33f, 0.917f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //I

         0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //C
         0.41f, -0.02f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, //D
        -0.157f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.0f, 0.8f, //L

        0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //C
        -0.157f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 0.0f, 0.8f, //L
        0.06f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, //G

        0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //C
        0.06f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.0f, 0.2f, //G
        0.76f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.7f, 0.0f, //E

        0.41f, 0.68f, 1.134f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, //C
        0.76f, 0.897f, 0.567f, 0.0f, 0.0f, 1.0f, 0.7f, 0.0f, //E
        0.977f, 0.33f, 0.217f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, //J

                //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
       //Positions          //Normals
        //Front Face         //Positive Z Normal
       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        //Left Face          //Negative X Normal
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Right Face         //Positive X Normal
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Bottom Face        //Negative Y Normal
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        //Top Face           //Positive Y Normal
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride =  sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh &mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint &textureId)
{
    int width, height, channels;
    unsigned char *image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
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

		glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
