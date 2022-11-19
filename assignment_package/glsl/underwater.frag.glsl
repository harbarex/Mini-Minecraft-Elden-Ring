#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    //background texture
    vec4 texture_color = texture(u_RenderedTexture, fs_UV);

    vec4 water_color = vec4(0.192156862745098, 0.6627450980392157, 0.9333333333333333, 1.0);

    vec4 k = vec4(u_Time)*0.025;
    k.xy = fs_UV * 7.0;
    float val1 = length(0.5-fract(k.xyw*=mat3(vec3(-2.0,-1.0,0.0), vec3(3.0,-1.0,1.0), vec3(1.0,-1.0,-1.0))*0.5));
    float val2 = length(0.5-fract(k.xyw*=mat3(vec3(-2.0,-1.0,0.0), vec3(3.0,-1.0,1.0), vec3(1.0,-1.0,-1.0))*0.2));
    float val3 = length(0.5-fract(k.xyw*=mat3(vec3(-2.0,-1.0,0.0), vec3(3.0,-1.0,1.0), vec3(1.0,-1.0,-1.0))*0.5));

    color = (vec4 ( pow(min(min(val1,val2),val3), 7.0) * 3.0)+texture_color*water_color).rgb;
}
