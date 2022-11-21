#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;
uniform sampler2D u_Texture;

in vec2 fs_UV;

out vec4 color;

float hash21(in vec2 n){ return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453); }
mat2 makem2(in float theta){float c = cos(theta);float s = sin(theta);return mat2(c,-s,s,c);}

vec2 random2(vec2 p){
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float noise(vec2 uv){
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0;    // Minimum distance initialized to max.
    float secondMinDist = 1.0;
    vec2 closestPoint;

    // Search all neighbouring cells and this cell for their point.
    for(int y = -1; y <= 1; y++){
        for(int x = -1; x <=1; x++){
            vec2 neighbour = vec2(float(x), float(y));

            // Random point inside current neighbouring cell
            vec2 point  = random2(uvInt + neighbour);

            // Compute the distance between point & fragment
            // Store the min dist so far
            vec2 diff   = neighbour + point - uvFract;
            float dist  = length(diff);

            if(dist < minDist){
                minDist     = dist;
                float r_t   = cos(float(u_Time*0.001));
                point.x = cos(r_t) * point.x + sin(r_t) * point.y;
                point.y = -sin(r_t) * point.x + cos(r_t) * point.y;
            }

        }
    }

    return cos(minDist * 3.14159265 * 0.7);
}

vec2 gradn(vec2 p)
{
        float ep = .09;
        float gradx = noise(vec2(p.x+ep,p.y))-noise(vec2(p.x-ep,p.y));
        float grady = noise(vec2(p.x,p.y+ep))-noise(vec2(p.x,p.y-ep));
        return vec2(gradx,grady);
}

float flow(in vec2 p)
{
        float z=2.;
        float rz = 0.;
        vec2 bp = p;
        for (float i= 1.;i < 7.;i++ )
        {
                //primary flow speed
                p += u_Time*0.001*.6;

                //secondary flow speed (speed of the perceived flow)
                bp += u_Time*0.001*1.9;

                //displacement field (try changing u_Time*0.1 multiplier)
                vec2 gr = gradn(i*p*.34+u_Time*0.001*1.);

                //rotation of the displacement field
                gr*=makem2(u_Time*0.001*6.-(0.05*p.x+0.03*p.y)*40.);

                //displace the system
                p += gr*.5;

                //add noise octave
                rz+= (sin(noise(p)*7.)*0.5+0.5)/z;

                //blend factor (blending displaced system with base system)
                //you could call this advection factor (.5 being low, .95 being high)
                p = mix(bp,p,.77);

                //intensity scaling
                z *= 1.4;
                //octave scaling
                p *= 2.;
                bp *= 1.9;
        }
        return rz;
}

void main()
{
        vec2 p = fs_UV-0.5;
        p.x *= u_Dimensions.x/u_Dimensions.y;
        p*= 3.;
        float rz = flow(p);

        vec3 col = vec3(.4,0.07,0.01)/rz;
        //color   = vec4(col, 1);
        color = vec4(texture(u_Texture, fs_UV).rgb + col, 1);
        //color = texture(u_RenderedTexture, fs_UV).rgb * pow(col,vec3(1.4));
}
