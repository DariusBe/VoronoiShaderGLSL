
#include <iostream>
#include <windows.h>
#include <GLEW/include/GL/glew.h>
#include <GLFW/include/GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

int screenWidth = 600; int screenHeight = 600;
double mouseX, mouseY;
unsigned int shader = 0;
float positions[8];


//simple struct "emulating" vec2 within shader code
struct vec2 {
    float x;
    float y;
};

void removePoints(std::vector<vec2>& inputPoints, float normalizedX, float normalizedY, float pickupDistance) {
    //prevent vector bounds exception: only iterate through vector to remove if list>1
    if (inputPoints.size() <= 1) {
        inputPoints.empty();
    }
    else {
        auto newEnd = std::remove_if(inputPoints.begin(), inputPoints.end(),
            [normalizedX, normalizedY, pickupDistance](const vec2& point) {
                return (std::abs(point.x - normalizedX) < pickupDistance) && (std::abs(point.y - normalizedY) < pickupDistance);
            });

        inputPoints.erase(newEnd, inputPoints.end());
    }
}

// Maximum number of mouse input points
const int maxInputPoints = 100;
std::vector<vec2> inputPoints; //points storage used within shader via uniform

string windowTitle = "Voronoi Shader - OpenGL";

//needed as return type in following ParseShader function:
//since we want to return two strings individually, we use this struct:
struct ShaderProgramSource {
    string VertexSource;
    string FragmentSource;
};

static ShaderProgramSource ParseShader(const string& filepath) {
    //ifstream: open from "input file"-stream
    ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    //in our shader file, we use "#shader [keyword]"
    //to distinguish component of our shader (vertex, fragment, ..)
    //so, our shader can be saparated based on these comments:
    string line;
    stringstream ss[2]; //1x for vertex, 1x for fragment shader component
    ShaderType shadertype = ShaderType::NONE;
    while (getline(stream, line)) {
        //if line contains "#shader" and it's not an invalid string position
        if (line.find("#shader") != string::npos) {
            //if so, is there "vertex" keyword?
            if (line.find("vertex") != string::npos) {
                //set mode to vertex
                shadertype = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != string::npos) {
                //set mode to fragment
                shadertype = ShaderType::FRAGMENT;
            }
        }
        //if there is another #shader [keyword]
        else {
            //for every other line, take line and put
            //it into ss[] with either index [0] => vertex or [1] => fragment
            ss[((int)shadertype)] << line << '\n';
        }
    }
    //return of the two strings via struct ShaderProgramSource:
    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const string& source) {
    unsigned int id = glCreateShader(type);
    //return pointer to data in string:
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //check if compilation was successfull
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile " <<
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << endl;
        cout << message << endl;

        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void createInitialPoints() {
    for (int i = 0; i < 25; i++) {
        vec2 newPoint;
        newPoint.x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        newPoint.y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        inputPoints.push_back(newPoint);
    }

    // Pass the updated points array to the shader as a uniform
    GLint pointsCountLoc = glGetUniformLocation(shader, "inputPointsCount");
    GLint pointsLoc = glGetUniformLocation(shader, "inputPoints");

    glUniform1i(pointsCountLoc, static_cast<int>(inputPoints.size()));
    glUniform2fv(pointsLoc, static_cast<GLsizei>(inputPoints.size()), reinterpret_cast<const float*>(&inputPoints[0]));
}

static void printStringWithLineNumbers(string str, int color) {

    //Tint Console Output Font via Handle
    HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);

    size_t lineNum = 1; // Initialize line number
    // Using iterators to split the string into lines
    size_t pos = 0;
    while (pos < str.length()) {
        size_t nextPos = str.find('\n', pos); // Find the next newline character
        if (nextPos == string::npos) {
            nextPos = str.length(); // If no newline found, set nextPos to the end
        }

        // Extract the substring between pos and nextPos (representing a line)
        std::string line = str.substr(pos, nextPos - pos);

        // Print line number and the line itself using printf
        printf("%i\t%s\n", lineNum++, line.c_str());

        // Move to the next position after the newline character (or end of the string)
        pos = nextPos + 1;
    }
    //set font color white again
    SetConsoleTextAttribute(hConsole, 15);
    printf("\n");
}

// GLFW mouse button callback function
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

    // Pass the updated points array to the shader as a uniform
    GLint pointsCountLoc = glGetUniformLocation(shader, "inputPointsCount");
    GLint pointsLoc = glGetUniformLocation(shader, "inputPoints");

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        // Get the mouse cursor position
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Normalize mouse coordinates to the range [-1, 1]
        // Assuming screen width and height are stored in screenWidth and screenHeight
        float normalizedX = mouseX / screenWidth;
        float normalizedY = 1.0 - (mouseY / screenHeight);

        if (inputPoints.size() < maxInputPoints) {
            vec2 newPoint;
            newPoint.x = normalizedX;
            newPoint.y = normalizedY;
            inputPoints.push_back(newPoint);
        }

        for (auto& element : inputPoints) {
            cout << "(" << element.x << ", " << element.y << ")" << "\t remaining: " << maxInputPoints - inputPoints.size() << endl;
        }

        // Pass mouse position to the shader
        GLint mousePosLoc = glGetUniformLocation(shader, "mousePos");
        glUseProgram(shader);
        glUniform2f(mousePosLoc, normalizedX, normalizedY);
        glUniform1i(pointsCountLoc, static_cast<int>(inputPoints.size()));
        glUniform2fv(pointsLoc, static_cast<GLsizei>(inputPoints.size()), reinterpret_cast<const float*>(&inputPoints[0]));

        string newWindowTitle = windowTitle + " - remaining addable points: " + to_string(maxInputPoints - inputPoints.size());
        glfwSetWindowTitle(window, newWindowTitle.data());
    }

    //With Right Mouse Button, Remove Points from list (by proximity):
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float normalizedX = mouseX / screenWidth;
        float normalizedY = 1.0 - (mouseY / screenHeight);

        float pickupDistance = 0.015f;
        removePoints(inputPoints, normalizedX, normalizedY, pickupDistance);

        for (auto& element : inputPoints) {
            cout << "(" << element.x << ", " << element.y << ")" << "\t remaining: " << maxInputPoints - inputPoints.size() << endl;
        }

        glUniform1i(pointsCountLoc, static_cast<int>(inputPoints.size()));
        glUniform2fv(pointsLoc, static_cast<GLsizei>(inputPoints.size()), reinterpret_cast<const float*>(&inputPoints[0]));

        string newWindowTitle = windowTitle + " - remaining addable points: " + to_string(maxInputPoints - inputPoints.size());
        glfwSetWindowTitle(window, newWindowTitle.data());
    }
}



int main(void)
{
    //initialize GLFW
    if (!glfwInit()) return -1;

    //set GLFW Object with used versions (here: 3.3)
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  //major version
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  //sub-version
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //forward-compat.: true
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);        //for window with static size

    //create new windows based on HEIGHT, WIDTH, name Window and
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, windowTitle.data(), NULL, NULL);

    /*
    //to get actual resolution for high-density displays:
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    */

    //print message, if screen has NOT been initialized succesfully:
    if (!window) {
        cout << "Failed to create GLFW Window" << endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    //set window to current context:
    glfwMakeContextCurrent(window);
    //enable GLEW:
    //glewExperimental = GL_TRUE;

    //GLEW initialization (test):
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        return EXIT_FAILURE;
    }

    //now lets create something and store it in the vbo:
    //position for vertices of a canvas:
    float positions[8] = {
        -1.0f,  1.0f, // Top-left corner
        -1.0f, -1.0f, // Bottom-left corner
         1.0f, -1.0f, // Bottom-right corner
         1.0f,  1.0f  // Top-right corner
    };

    //this data has no meaning so far
    //nor does GL know which of those are x or y

    //[COMING BACK FROM OUR draw Viewport stuff below]
    //to create something, like, say, an object:
    //create vertex vbo for data to be displayed
    GLuint vbo; //id
    glGenBuffers(1, &vbo);
    /*  created a vertex vbo identitiable by a id number
        this vbo is then used (in while loop) to render
        vertex with shaders */

        // binding => selecting this vbo for specific task
    glBindBuffer(GL_ARRAY_BUFFER, vbo);


    //so what's needed? => Data Layout
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
    //https://docs.gl/gl4/glVertexAttribPointer
    //needs to be enabled too:
    glEnableVertexAttribArray(0);

    //shader's still missing; so here, finally:
    //shader comes from a external file


    ShaderProgramSource source = ParseShader("libraries/OpenGL/shader/Basic.shader");
    cout << "Vertex Shader:" << endl;
    printStringWithLineNumbers(source.VertexSource, 6);     //6 = yellow

    cout << "Fragment Shader:" << endl;
    printStringWithLineNumbers(source.FragmentSource, 3);   //3 = blue

    // Use Shader:
    shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    //Provide Shader with Uniform variable from Screen Size (see use within shader code!):
    // Get the window width and height from GLFW
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    // Use glGetUniformLocation to get the location of the uniform variable in the shader program
    int resolutionLocation = glGetUniformLocation(shader, "resolution");
    // Set the resolution in the shader
    glUniform2f(resolutionLocation, (float)screenWidth, (float)screenHeight);



    // Set the mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);




    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);
    //https://docs.gl/gl3/glBufferData 
    //GL_STATIC_DRAW sends hint to GL: 
    //data store contents will be modified once (upon creation)
    //and will be used many times (every frame)

    //now OpenGL has coordinates and a vbo, but what to do with that?
    //clarify, how our data is layed out (necessary for shader to process data)

    createInitialPoints();

    //draw Viewport in top left corner
    //glViewport(0, 0, screenWidth, screenHeight);
    //as long as window is not closed:
    //HERE's THE ACTION!
    while (!glfwWindowShouldClose(window)) {
        //render:
        //blank viewport
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //takes vbo from above (automatically, because of binding)
        // if I created another vbo and bound it to, say, 0, the data
        // would not match our triangle here:
        glDrawArrays(GL_QUADS, 0, 4);
        //https://docs.gl/gl3/glDrawArrays (0=offset, 3 vertexes from vbo);

        //draw OpenGL
        glfwSwapBuffers(window);

        //poll for and process events
        glfwPollEvents();
    }


    //glDeleteProgram(shader);

    return EXIT_SUCCESS;
}

