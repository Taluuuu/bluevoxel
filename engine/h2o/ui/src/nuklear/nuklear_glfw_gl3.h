#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "nuklear_headers.h"

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif

struct nk_glfw_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
};

struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_glfw_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
};

NK_API struct nk_context*   nk_glfw3_init(struct nk_glfw* glfw, GLFWwindow *win, enum nk_glfw_init_state);
NK_API void                 nk_glfw3_shutdown(struct nk_glfw* glfw);
NK_API void                 nk_glfw3_font_stash_begin(struct nk_glfw* glfw, struct nk_font_atlas **atlas);
NK_API void                 nk_glfw3_font_stash_end(struct nk_glfw* glfw);
NK_API void                 nk_glfw3_new_frame(struct nk_glfw* glfw);
NK_API void                 nk_glfw3_render(struct nk_glfw* glfw, enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

NK_API void                 nk_glfw3_device_destroy(struct nk_glfw* glfw);
NK_API void                 nk_glfw3_device_create(struct nk_glfw* glfw);

NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
NK_API void                 nk_glfw3_mouse_button_callback(GLFWwindow *win, int button, int action, int mods);