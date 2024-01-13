#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

const char *shaderv_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 vert_pos;"
    "void main() {"
    "   gl_Position = vec4(vert_pos, 0.0, 1.0);"
    "}\0";
const char *shaderf_src =
    "#version 330 core\n"
    "out vec4 frag_col;"
    "void main() {"
    "   frag_col = vec4(vec3(0.9451), 1.0);"
    "}\0";

int main(){
    GLFWwindow *window;
    GLuint      shaderv, shaderf, shaderp;
    GLuint      VAO, VBO, EBO;
    GLuint      indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    GLfloat     vertices[] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    window = glfwCreateWindow(512, 512, "[ piiota ]", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    shaderv = glCreateShader(GL_VERTEX_SHADER);
    shaderf = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderv, 1, &shaderv_src, NULL);
    glShaderSource(shaderf, 1, &shaderf_src, NULL);
    glCompileShader(shaderv);
    glCompileShader(shaderf);

    shaderp = glCreateProgram();
    glAttachShader(shaderp, shaderv);
    glAttachShader(shaderp, shaderf);
    glLinkProgram(shaderp);

    glDeleteShader(shaderv);
    glDeleteShader(shaderf);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.375, 0.375, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderp);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderp);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}