#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glee.h>
#include <photon.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

vec2 mouse, resolution, cam;
int mouseHand = 0;
float scale = 1.0;

vec3 rand_position()
{
#define RAD_MIN 10.0
#define RAD_MAX 20.0
    float z = randf_norm() * RAD_MAX + RAD_MIN;
    vec3 v = {
        randf_norm() * (resolution.x - 2.0 * z) + z,
        randf_norm() * (resolution.y - 2.0 * z) + z,
        z
    };
    return v;
}

vec4 rand_color()
{
    vec4 c = {
        randf_norm(),
        randf_norm(),
        randf_norm(),
        randf_norm()
    };
    return c;
}

vec4 color_neg(vec4 color)
{
    vec4 c = {
        1.0 - color.x,
        1.0 - color.y,
        1.0 - color.z, 
        1.0
    };
    return c;
}

vec2 vec2_lerp(vec2 v1, vec2 v2)
{
    vec2 v = {lerpf(v1.x, v2.x, 0.5), lerpf(v1.y, v2.y, 0.5)};
    return v;
}

void spawn_circles(const unsigned int count, vec3* positions, vec4* colors, vec2* velocities)
{
    cam = vec2_uni(0.0);
    for (unsigned int i = 0; i < count; i++) {
        velocities[i] = vec2_uni(0.0);
        colors[i] = rand_color();
        positions[i] = rand_position();
        Circle c1 = *(Circle*)&positions[i];
        for (int j = 0; j < i; j++) {
            if (i == j) continue;
            Circle c2 = *(Circle*)&positions[j];
            while (circle_overlap(c1, c2)) {
                positions[i] = rand_position();
                c1 = *(Circle*)&positions[i];
            }
        }
    }
}

void update(const unsigned int count, unsigned int shader, vec4* colors, vec3* positions, vec2* velocities, int* marked, float deltaTime)
{
    unsigned int mouseDown = glee_mouse_down(GLFW_MOUSE_BUTTON_LEFT);
    vec2 mMouse = _vec2_div(mouse, scale);
    mMouse = vec2_add(mMouse, cam);

    for (unsigned int i = 0; i < count; i++) {

        if (circle_point_overlap(*(Circle*)&positions[i], mMouse)) {
            if (!marked[i]) {
                colors[i] = color_neg(colors[i]);
                marked[i] = 1;
            }
            if (mouseDown) {
                velocities[i] = vec2_mult(vec2_sub(mMouse, *(vec2*)&positions[i]), 10.0);
                positions[i].x = mMouse.x;
                positions[i].y = mMouse.y;
                mouseHand = 1;
            } else mouseHand = 0;
        } else if (marked[i]) {
            colors[i] = color_neg(colors[i]);
            marked[i] = 0;
        }
        
        vec2 dif = {
            positions[i].x + velocities[i].x * deltaTime,
            positions[i].y + velocities[i].y * deltaTime
        };

        for (unsigned int j = 0; j < count; j++) {
            if (i == j) continue;

            //Apply gravity force to each velocity
#define G_FORCE 0.2f
            float m = positions[i].z * positions[j].z;
            float r = vec2_dist(*(vec2*)&positions[i], *(vec2*)&positions[j]);
            float f = (G_FORCE * m) / r;
            vec2 g = vec2_norm(vec2_sub(*(vec2*)&positions[j], *(vec2*)&positions[i]));
            velocities[i] = vec2_add(velocities[i], vec2_mult(g, f));

            //Collisions
            vec2 vdif = vec2_sub(
                vec2_mult(velocities[i], deltaTime), 
                vec2_mult(velocities[j], deltaTime)
            );

            if (circle_overlap_offset(*(Circle*)&positions[i], *(Circle*)&positions[j], vdif)) {
                float M = positions[i].z + positions[j].z;
                vec2 v1 = velocities[i], v2 = velocities[j];
                velocities[i].x = ((positions[i].z - positions[j].z) / M) * v1.x + ((2.0 * positions[j].z) / M) * v2.x;
                velocities[i].y = ((positions[i].z - positions[j].z) / M) * v1.y + ((2.0 * positions[j].z) / M) * v2.y;
                float mag1 = vec2_mag(velocities[i]);
                vec2 dir1 = vec2_mult(g, -mag1);

                velocities[j].x = ((positions[j].z - positions[i].z) / M) * v2.x + ((2.0 * positions[i].z) / M) * v1.x;
                velocities[j].y = ((positions[j].z - positions[i].z) / M) * v2.y + ((2.0 * positions[i].z) / M) * v1.y;
                float mag2 = vec2_mag(velocities[j]);
                vec2 dir2 = vec2_mult(g, mag2);

                velocities[i] = vec2_mult(vec2_norm(vec2_lerp(dir1, velocities[i])), mag1);
                velocities[j] = vec2_mult(vec2_norm(vec2_lerp(dir2, velocities[j])), mag2);
            }

            while(circle_overlap(*(Circle*)&positions[i], *(Circle*)&positions[j])) {
                positions[i].x -= g.x;
                positions[i].y -= g.y;
            }
        }

        // Apply velocity to positions
#define MAX_VEL 400.0f
        velocities[i].x = clampf(velocities[i].x, -MAX_VEL, MAX_VEL);
        velocities[i].y = clampf(velocities[i].y, -MAX_VEL, MAX_VEL);
        positions[i].x += velocities[i].x * deltaTime;
        positions[i].y += velocities[i].y * deltaTime;

        //Draw the circles
        vec3 pos = {
            (positions[i].x - cam.x) * scale,
            (positions[i].y - cam.y) * scale,
            positions[i].z * scale
        };
        glee_shader_uniform_set(shader, 3, "u_pos", &pos);
        glee_shader_uniform_set(shader, 4, "u_color", &colors[i]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

int main(int argc, char** argv)
{
    rand_seed(time(NULL));
    unsigned int fullscreen = 0, count = rand_next() % 20 + 5;
    
    if (argc > 1) {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help")) {
            printf("----------------- GRAVITY 2D ------------------\n");
            printf("gravity2D is a simple, interactive screen-saver\n");
            printf("use options -f or -fullscreen for maximum fun.\n");
            printf("\t\tversion 0.1.1\n");
            return EXIT_SUCCESS;
        }
        else if (!strcmp(argv[1], "-f") || !strcmp(argv[1], "-fullscreen")) {
            fullscreen++;
            if (argc > 2) count = atoi(argv[2]);
        } else {
            count = atoi(argv[1]);
            if (!count) count = 2;
        }
    } 

    if (fullscreen) {
        resolution.x = 1440.0;
        resolution.y = 960.0;
    } else {
        resolution.x = (float)SCR_WIDTH;
        resolution.y = (float)SCR_HEIGHT;
    }

    printf("Resolution: %dx%d\n", (int)resolution.x, (int)resolution.y);

    glee_init();
    glee_window_create("gravity2D", (int)resolution.x, (int)resolution.y, fullscreen, 0);
    glee_screen_color(0.0, 0.0, 0.0, 1.0);

    unsigned int quad = glee_buffer_quad_create();
    unsigned int shader_back = glee_shader_load("shaders/vert.glsl", "shaders/frag.glsl");
    glee_shader_uniform_set(shader_back, 2, "u_resolution", &resolution);
    unsigned int shader_circle = glee_shader_load("shaders/vert.glsl", "shaders/circle.glsl");
    glee_shader_uniform_set(shader_circle, 2, "u_resolution", &resolution);
    
    vec2 velocities[count];
    vec3 positions[count];
    vec4 colors[count];
    spawn_circles(count, &positions[0], &colors[0], &velocities[0]);

    int marked[count];
    for (int i = 0; i < count; i ++) marked[i] = 0;

    vec2 mouseMark = {0.0, 0.0};

    float deltaTime, t = glee_time_get();
    while(glee_window_is_open()) {
        glee_screen_clear();
        deltaTime = glee_time_delta(&t);
        printf("delta time: %f\r", deltaTime);
        
        // Input
#define SPEED 10.0
        glee_mouse_pos(&mouse.x, &mouse.y);
        if (glee_key_pressed(GLFW_KEY_ESCAPE)) break;
        if (glee_key_pressed(GLFW_KEY_R)) spawn_circles(count, &positions[0], &colors[0], &velocities[0]);
        if (glee_key_down(GLFW_KEY_Z)) scale += deltaTime;
        else if (glee_key_down(GLFW_KEY_X)) scale -= deltaTime;
        
        if (glee_key_down(GLFW_KEY_W)) cam.y += SPEED * deltaTime * scale;
        if (glee_key_down(GLFW_KEY_S)) cam.y -= SPEED * deltaTime * scale;
        if (glee_key_down(GLFW_KEY_D)) cam.x += SPEED * deltaTime * scale;
        if (glee_key_down(GLFW_KEY_A)) cam.x -= SPEED * deltaTime * scale;
        
        unsigned int mouseDown = glee_mouse_down(GLFW_MOUSE_BUTTON_LEFT);
        if (glee_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) mouseMark = mouse;
        if (mouseDown && !mouseHand) {
            cam.x -= (mouse.x - mouseMark.x);
            cam.y -= (mouse.y - mouseMark.y);
            mouseMark = mouse;
        }

        glUseProgram(shader_back);
        glee_shader_uniform_set(shader_back, 2, "u_mouse", &mouse);
        glee_shader_uniform_set(shader_back, 1, "u_time", &t);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glUseProgram(shader_circle);
        update(count, shader_circle, &colors[0], &positions[0], &velocities[0], &marked[0], deltaTime);

        glee_screen_refresh();
    }
    glee_deinit();
    return 0;
}
