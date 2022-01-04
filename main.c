#include <stdio.h>
#include <stdlib.h>
#include <glee.h>
#include <photon.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

vec2 mouse, resolution;

vec3 rand_position()
{
    float z = randf_norm() * 50.0f + 10.0f;
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

void spawn_circles(const unsigned int count, vec3* positions, vec4* colors, vec2* velocities)
{
    for (int i = 0; i < count; i++) {
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

    for (int i = 0; i < count; i++) {

        if (circle_point_overlap(*(Circle*)&positions[i], mouse)) {
            if (!marked[i]) {
                colors[i] = color_neg(colors[i]);
                marked[i] = 1;
            }
            if (mouseDown) {
                velocities[i] = vec2_mult(vec2_sub(mouse, *(vec2*)&positions[i]), 10.0);
                positions[i].x = mouse.x;
                positions[i].y = mouse.y;
            }
        } else if (marked[i]) {
            colors[i] = color_neg(colors[i]);
            marked[i] = 0;
        }
        
        vec2 dif = {
            positions[i].x + velocities[i].x * deltaTime,
            positions[i].y + velocities[i].y * deltaTime
        };

        for (int j = 0; j < count; j++) {
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
                velocities[i] = vec2_mult(vec2_reflect(velocities[i], g), clampf(positions[j].z / positions[i].z, 0.0, 1.0));
                velocities[j] = vec2_mult(vec2_reflect(velocities[j], g), clampf(positions[i].z / positions[j].z, 0.0, 1.0));
                //velocities[i] = vec2_add(velocities[i], vec2_mult(vec2_reflect(velocities[i], g), maxf(positions[j].z / positions[i].z, 1.0)));
                //velocities[j] = vec2_add(velocities[j], vec2_mult(vec2_reflect(velocities[j], g), maxf(positions[i].z / positions[j].z, 1.0)));
            }

            while(circle_overlap(*(Circle*)&positions[i], *(Circle*)&positions[j])) {
                positions[i].x -= g.x;
                positions[i].y -= g.y;
                //velocities[i] = vec2_add(velocities[i], vec2_mult(g, 1.0));
            }
        }

        // Collide with screen walls
        if (dif.x > resolution.x - positions[i].z || 
            dif.x < positions[i].z) velocities[i].x = -velocities[i].x;
        if (dif.y > resolution.y - positions[i].z ||
            dif.y < positions[i].z) velocities[i].y = -velocities[i].y;

        // Apply velocity to positions
#define MAX_VEL 400.0f
        velocities[i].x = clampf(velocities[i].x, -MAX_VEL, MAX_VEL);
        velocities[i].y = clampf(velocities[i].y, -MAX_VEL, MAX_VEL);
        positions[i].x += velocities[i].x * deltaTime;
        positions[i].y += velocities[i].y * deltaTime;

        //Draw the circles
        glee_shader_uniform_set(shader, 3, "u_pos", &positions[i]);
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
    glee_window_create("gravity2D", resolution.x, resolution.y, fullscreen, 0);
    glee_screen_color(0.0, 0.0, 0.0, 1.0);

    vec2 res = resolution;
    glee_buffer_quad_create();
    unsigned int shader_back = glee_shader_load("shaders/vert.glsl", "shaders/frag.glsl");
    glee_shader_uniform_set(shader_back, 2, "u_resolution", &res);
    unsigned int shader_circle = glee_shader_load("shaders/circlev.glsl", "shaders/circlef.glsl");
    glee_shader_uniform_set(shader_circle, 2, "u_resolution", &res);
    
    int marked[count];
    for (int i = 0; i < count; i ++) marked[i] = 0;
    vec2 velocities[count];
    vec3 positions[count];
    vec4 colors[count];
    spawn_circles(count, &positions[0], &colors[0], &velocities[0]);

    float deltaTime, t = glee_time_get();
    while(glee_window_is_open()) {
        glee_screen_clear();
        
        if (glee_key_pressed(GLFW_KEY_ESCAPE)) break;
        if (glee_key_pressed(GLFW_KEY_R)) spawn_circles(count, &positions[0], &colors[0], &velocities[0]);

        glee_mouse_pos(&mouse.x, &mouse.y);
        deltaTime = glee_time_delta(&t);
        printf("delta time: %f\r", deltaTime);

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
