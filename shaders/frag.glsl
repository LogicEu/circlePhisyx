out vec4 FragColor;

uniform float u_time;
uniform vec2 u_resolution;
uniform vec2 u_mouse;

void main() 
{
	vec2 uv = gl_FragCoord.xy / u_resolution.y;
    vec2 mouse = 2. * u_mouse / u_resolution.y;

    float n = 0.0;
    if (length(mouse - uv) < 0.2) n = 1.0;
	vec3 color = vec3(uv.x, uv.y, cos(u_time) + n);
	FragColor = vec4(color, 1.0);
}