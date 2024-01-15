#include <stdio.h>
#include <math.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>


#define BOX_RADIUS        20.0f
#define BOX_SHADOW_LENGTH 2.0f
#define N_CELLS_X         15
#define N_CELLS_Y         15
#define CELL_PADDING      4
#define MARGIN            30

#define WINDOW_HEIGHT     600
#define WINDOW_WIDHT      ((6 * WINDOW_HEIGHT - 3 * MARGIN) / 5)

#define COLOR_BACKGROUND ((float[]) {0.4f, 0.4f, 1.0f})
#define COLOR_FOOD       ((float[]) {1.0f, 0.4f, 0.4f})
#define COLOR_HEAD       ((float[]) {0.2f, 0.2f, 0.5f})
#define COLOR_TAIL       ((float[]) {0.4f, 1.0f, 0.4f})
#define COLOR_FONT1      ((float[]) {0.9451f, 0.9451f, 0.9451f})
#define COLOR_FONT2      ((float[]) {0.5922f, 0.5922f, 0.5922f})
#define COLOR_BOX        ((float[]) {0.1412f, 0.1608f, 0.1843f})

const char *shaderv_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 vert_pos;"
    "uniform vec2 window_size;"
    "uniform vec2 frambuffer_size;"
    "uniform vec2 obj_pos;"
    "uniform vec2 obj_size;"
    "uniform vec3 obj_color;"
    "uniform float radius;"
    "uniform float shadow_length;"
    "void main() {"
    "   vec2 nvert_pos = vert_pos;"
    "   nvert_pos *= obj_size / window_size;"
    "   nvert_pos += obj_pos / window_size;"
    "   nvert_pos *= 2.0;"
    "   nvert_pos -= vec2(1.0, 1.0);"
    "   gl_Position = vec4(nvert_pos, 0.0, 1.0);"
    "}\0";
const char *shaderf_src =
    "#version 330 core\n"
    "uniform vec2 window_size;"
    "uniform vec2 frambuffer_size;"
    "uniform vec2 obj_pos;"
    "uniform vec2 obj_size;"
    "uniform vec3 obj_color;"
    "uniform float radius;"
    "uniform float shadow_length;"
    "out vec4 frag_col;"
    "void main() {"
    "   frag_col = vec4(obj_color, 1.0);"
    "}\0";


struct rectangle {
    GLfloat pos_x, pos_y;
    GLfloat size_w, size_h;
    GLfloat shadow_length;
};
struct field {
    GLint   nx, ny;
    GLfloat padding;
    GLfloat margin;
};

struct snake {
    int head;
    int tail[N_CELLS_X * N_CELLS_Y - 1];
    int tail_length;
};

int main(){
    GLint       index;
    GLFWwindow *window;
    GLuint      shaderv, shaderf, shaderp;
    GLuint      VAO, VBO, EBO;
    GLint       w_sizew, w_sizeh;
    GLint       fb_sizew, fb_sizeh;
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
    struct rectangle box_title;
    struct rectangle box_label[3];
    struct rectangle box_field;
    struct rectangle cell;
    GLfloat          margin;
    GLint            field[N_CELLS_X * N_CELLS_Y];
    struct snake     snake;
    GLint            food;

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "[ piiota ]", NULL, NULL);
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

    margin = (GLfloat) MARGIN;
    box_title.size_w = (GLfloat) (WINDOW_WIDHT - 2 * (GLint) margin);
    box_title.size_h = (GLfloat) (WINDOW_HEIGHT - 3.0f * margin) / 5.0f;
    box_title.pos_x  = margin;
    box_title.pos_y  = (GLfloat) WINDOW_HEIGHT - box_title.size_h - margin;
    box_title.shadow_length = 0.0f;

    box_field.size_w = ((GLfloat) (WINDOW_WIDHT - 3 * (GLint) margin)) * 2.0f / 3.0f;;
    box_field.size_h = (GLfloat) (WINDOW_HEIGHT - 3.0f * margin) * 4.0f / 5.0f;
    box_field.pos_x  = margin;
    box_field.pos_y  = margin;
    box_field.shadow_length = 0.0f;

    for (index = 0; index < 3; index++) {
        box_label[index].size_w = box_field.size_h / 2.0f;
        box_label[index].size_h = (box_field.size_h - 2.0f * margin) / 3.0f;
        box_label[index].pos_x  = box_field.pos_x + box_field.size_w + margin;
        box_label[index].pos_y  = margin + index * (box_label[index].size_h + margin);
        box_label[index].shadow_length = 0.0f;
    }

    box_field.size_w = ((GLfloat) (WINDOW_WIDHT - 3 * (GLint) margin)) * 2.0f / 3.0f;
    box_field.size_h = box_title.size_h * 4.0f;
    box_field.pos_x  = margin;
    box_field.pos_y  = margin;
    box_field.shadow_length = 0.0f;

    cell.size_w = (box_field.size_w - 2.0f * (margin + (GLfloat) BOX_RADIUS) - (GLfloat)(N_CELLS_X - 1) * CELL_PADDING) / N_CELLS_X;
    cell.size_h = (box_field.size_h - 2.0f * (margin + (GLfloat) BOX_RADIUS) - (GLfloat)(N_CELLS_Y - 1) * CELL_PADDING) / N_CELLS_Y;
    cell.pos_x  = box_field.pos_x + margin + (GLfloat) BOX_RADIUS;
    cell.pos_y  = box_field.pos_y + margin + (GLfloat) BOX_RADIUS;
    cell.shadow_length = 0.0f;


    for (index = 0; index < N_CELLS_X * N_CELLS_Y; index++) {
        field[index] = -1;

        /* TODO: calc circle field */
        /* CENTER: N_CELLS_X / 2, N_CELLS_Y / 2 * N_CELLS */
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(COLOR_BACKGROUND[0], COLOR_BACKGROUND[1], COLOR_BACKGROUND[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwGetFramebufferSize(window, &fb_sizew, &fb_sizeh);
        glfwGetWindowSize(window, &w_sizew, &w_sizeh);

        /* rectangle shader */
        glUseProgram(shaderp);
        glUniform2f(glGetUniformLocation(shaderp, "window_size"), (GLfloat) WINDOW_WIDHT, (GLfloat) WINDOW_HEIGHT);
        glUniform2f(glGetUniformLocation(shaderp, "frambuffer_size"), (GLfloat) fb_sizew, (GLfloat) fb_sizeh);
        glUniform1f(glGetUniformLocation(shaderp, "radius"), BOX_RADIUS);
        glUniform1f(glGetUniformLocation(shaderp, "shadow_length"), BOX_SHADOW_LENGTH);
        glUniform3f(glGetUniformLocation(shaderp, "obj_color"), (GLfloat) COLOR_BOX[0], (GLfloat) COLOR_BOX[1], (GLfloat) COLOR_BOX[2]);

        /* title box */
        glUniform2f(glGetUniformLocation(shaderp, "obj_pos"), box_title.pos_x, box_title.pos_y);
        glUniform2f(glGetUniformLocation(shaderp, "obj_size"), box_title.size_w, box_title.size_h);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);

        /* field box */
        glUniform2f(glGetUniformLocation(shaderp, "obj_pos"), box_field.pos_x, box_field.pos_y);
        glUniform2f(glGetUniformLocation(shaderp, "obj_size"), box_field.size_w, box_field.size_h);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);

        /* label box */
        for (index = 0; index < 3; index++) {
            glUniform2f(glGetUniformLocation(shaderp, "obj_pos"), box_label[index].pos_x, box_label[index].pos_y);
            glUniform2f(glGetUniformLocation(shaderp, "obj_size"), box_label[index].size_w, box_label[index].size_h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);
        }
        /* cells */
        glUniform3f(glGetUniformLocation(shaderp, "obj_color"), (GLfloat) COLOR_FONT1[0], (GLfloat) COLOR_FONT1[1], (GLfloat) COLOR_FONT1[2]);
        for (index = 0; index < N_CELLS_X * N_CELLS_Y; index++) {
            glUniform2f(glGetUniformLocation(shaderp, "obj_pos"),
                cell.pos_x + (GLfloat) (index % N_CELLS_X) * cell.size_w + (GLfloat) (index % N_CELLS_X) * (GLfloat) CELL_PADDING,
                cell.pos_y + (GLfloat) (index / N_CELLS_X) * cell.size_h + (GLfloat) (index / N_CELLS_X) * (GLfloat) CELL_PADDING);
            glUniform2f(glGetUniformLocation(shaderp, "obj_size"), cell.size_w, cell.size_h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);
        }

        /* font shader */

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