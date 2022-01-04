out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec3 u_pos;
uniform vec4 u_color;

void main() 
{
	vec2 uv = gl_FragCoord.xy / u_resolution.y;
    vec3 pos = 2.0 * u_pos / u_resolution.y;
    
    float m = clamp(smoothstep(pos.z, length(pos.xy - uv), 0.01), 0.0, 1.0);
    float n = clamp(smoothstep(pos.z, length(pos.xy - uv), 0.59), 0.0, 1.0);

    vec4 col = vec4(u_color.xyz, 0.0);
    col += vec4(u_color.xyz, m);
    FragColor = col;
}
