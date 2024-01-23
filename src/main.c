#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define N_CELLS_X         16
#define N_CELLS_Y         16

#define WINDOW_HEIGHT     512
#define WINDOW_WIDHT      512

#define COLOR_BACKGROUND ((float[]) {0.4f, 0.4f, 1.0f})
#define COLOR_FOOD       ((float[]) {1.0f, 0.4f, 0.4f})
#define COLOR_HEAD       ((float[]) {0.3f, 0.75f, 0.3f})
#define COLOR_TAIL       ((float[]) {0.4f, 1.0f, 0.4f})
#define COLOR_FONT1      ((float[]) {0.9451f, 0.9451f, 0.9451f})
#define COLOR_FONT2      ((float[]) {0.5922f, 0.5922f, 0.5922f})
#define COLOR_BOX        ((float[]) {0.1412f, 0.1608f, 0.1843f})
#define COLOR_SHADOW     ((float[]) {0.1412f, 0.1608f, 0.1843f})

const char *shaderv_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 vert_pos;"
    "uniform vec2 grid_size;"
    "uniform float cell_id;"
    "void main() {"
    "   vec2 cell_size = vec2(1.0, 1.0) / grid_size;"

    "   float posx, posy;"
    "   posx = mod(cell_id, grid_size.x);"
    "   posy = floor(cell_id / grid_size.x);"

    "   vec2 new_vert = vert_pos;"
    "   new_vert = new_vert * cell_size;"
    "   new_vert = new_vert + vec2(posx, posy) * cell_size;"
    "   new_vert = new_vert * 2 - 1.0;"
    //"   new_vert = new_vert + (new_vert * vec2(posx, posy));"
    //"   new_vert = new_vert + new_vert * vec2(posx, posy);"

    "   gl_Position = vec4(new_vert, 0.0, 1.0);"
    "   "
    "}\0";
const char *shaderf_src =
    "#version 330 core\n"
    "out vec4 frag_col;"
    "uniform int color_value;"
    "uniform vec3 color_food;"
    "uniform vec3 color_head;"
    "uniform vec3 color_tail;"
    "uniform vec3 color_empty;"
    "void main() {"
    "   vec3 colors[4];"
    "   colors[0] = color_empty;"
    "   colors[1] = color_head;"
    "   colors[2] = color_tail;"
    "   colors[3] = color_food;"
    "   frag_col = vec4(colors[color_value], 1.0);"
    //"   frag_col = vec4(1.0, 0.4, 0.4, 1.0);"
    "}\0";

struct snake {
    size_t head_posx, head_posy;
    size_t length;
    size_t tail_posx;
    size_t tail_posy;
};

int main(){
    size_t      index;
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
    GLint field[N_CELLS_X * N_CELLS_Y] = {0};
    struct snake snake = {0};

    /* set random generator seed */
    srand((unsigned int) time(NULL));

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "[ piiota ]", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

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
        glClearColor(COLOR_BACKGROUND[0], COLOR_BACKGROUND[1], COLOR_BACKGROUND[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        if(glfwGetKey(window, GLFW_KEY_R)) {
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 0;
            snake.head_posx = (size_t) rand() % N_CELLS_X;
            snake.head_posy = (size_t) rand() % N_CELLS_Y;
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_LEFT)) {
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 0;
            if(snake.head_posx > 0) snake.head_posx -= 1;
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 0;
            if(snake.head_posx < N_CELLS_X - 1) snake.head_posx += 1;
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_DOWN)) {
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 0;
            if(snake.head_posy > 0) snake.head_posy -= 1;
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_UP)) {
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 0;
            if(snake.head_posy < N_CELLS_Y - 1) snake.head_posy += 1;
            field[snake.head_posx + snake.head_posy * N_CELLS_X] = 1;
        }


        glUseProgram(shaderp);
        glUniform2f(glGetUniformLocation(shaderp, "grid_size"), (GLfloat) N_CELLS_X, (GLfloat) N_CELLS_Y);
        glUniform3f(glGetUniformLocation(shaderp, "color_food"), COLOR_FOOD[0], COLOR_FOOD[1], COLOR_FOOD[2]);
        glUniform3f(glGetUniformLocation(shaderp, "color_head"), COLOR_HEAD[0], COLOR_HEAD[1], COLOR_HEAD[2]);
        glUniform3f(glGetUniformLocation(shaderp, "color_tail"), COLOR_TAIL[0], COLOR_TAIL[1], COLOR_TAIL[2]);
        glUniform3f(glGetUniformLocation(shaderp, "color_empty"), COLOR_BACKGROUND[0], COLOR_BACKGROUND[1], COLOR_BACKGROUND[2]);
        for(index = 0; index < N_CELLS_X * N_CELLS_Y; index++) {
            glUniform1f(glGetUniformLocation(shaderp, "cell_id"), (GLfloat) index);
            glUniform1i(glGetUniformLocation(shaderp, "color_value"), field[index]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);
        }

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