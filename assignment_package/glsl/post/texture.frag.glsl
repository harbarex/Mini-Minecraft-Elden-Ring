#version 150

uniform sampler2D u_Texture;
in vec2 fs_UV;
out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    //background texture
    vec4 texture_color  = texture(u_Texture, fs_UV);
    out_Col = texture_color;
}
