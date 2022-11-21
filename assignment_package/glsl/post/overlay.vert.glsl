#version 150

// Standard template Vertex Shader for overlay

in vec4 vs_Pos;
in vec4 vs_Col;
in vec2 vs_UV;

out vec4 fs_Col;
out vec2 fs_UV;

void main()
{
    fs_Col      = vs_Col;
    fs_UV       = vs_UV;
    gl_Position = vs_Pos;
}
