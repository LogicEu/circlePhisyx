layout (location = 0) in vec2 vertCoord;

out vec2 fragCoord;

void main()
{
    fragCoord = vertCoord;
    gl_Position = vec4(vertCoord, 0.0, 1.0);
}
