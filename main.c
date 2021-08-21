#include <stdio.h>
#include <glee.h>
#include <fract.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

int main(void)
{
    glee_init();
    glee_window_create("gravity2D", SCR_WIDTH, SCR_HEIGHT, 0, 0);
    glee_buffer_quad_create();
    unsigned int shader = glee_shader_load("shaders/vert.glsl", "shaders/frag.glsl");

    float time;
    vec2 resolution = {SCR_WIDTH, SCR_HEIGHT}, mouse;
    glee_shader_uniform_set(shader, 2, "u_resolution", &resolution);

    glee_screen_color(1.0, 0.0, 1.0, 1.0);
    while(glee_window_is_open()) {
        glee_screen_clear();
        if (glee_key_pressed(GLFW_KEY_ESCAPE)) break;

        glee_mouse_pos(&mouse.x, &mouse.y);
        time = glee_time_get();

        glee_shader_uniform_set(shader, 2, "u_mouse", &mouse);
        glee_shader_uniform_set(shader, 1, "u_time", &time);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glee_screen_refresh();
    }
    glee_deinit();
    return 0;
}
