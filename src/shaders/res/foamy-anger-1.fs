#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel0;

#define ch0 iChannel0

#define PI 3.14159265
#define fluid_rho 2.5
#define pixel_scale 4.0
#define particle_rad 1.0
#define dot2(x) dot(x,x)
#define range(i,a,b) for(int i = a; i <= b; i++)
#define Bi(p) ivec2(p)
#define texel(a, p) texelFetch(a, Bi(p), 0)

vec2 R;
vec4 Mouse;
float time;

struct particle {
    vec2 X;
    vec2 NX;
    float R;
    float M;
};

vec2 decode(float x) {
    uint X = floatBitsToUint(x);

    //return unpackSnorm2x16(X);
    return unpackHalf2x16(X);
}

particle getParticle(vec4 data, vec2 pos) {
    particle P;
    P.X = decode(data.x) + pos;
    P.NX = decode(data.y) + pos;
    P.R = data.z;
    P.M = data.w;
    return P;
}

float W(vec2 r, float h) {
    return (length(r / pixel_scale) >= 0.0 && h >= length(r / pixel_scale)) ? //>=
    (4.0 / (PI * pow(h, 8.0))) * pow(h * h - dot2(r / pixel_scale), 3.0) : 0.0;
}

vec2 WS(vec2 r, float h) {
    return (length(r / pixel_scale) > 0.0 && h >= length(r / pixel_scale)) ? -(30.0 / (PI * pow(h, 5.0))) * pow(h - length(r / pixel_scale), 2.0) * normalize(r) : vec2(0.0);
}

vec2 vel(particle P) {
    return P.NX - P.X;
}

vec3 distribution(vec2 x, vec2 p, vec2 K) {
    vec2 omin = clamp(x - K * 0.5, p - 0.5 * K, p + 0.5 * K);
    vec2 omax = clamp(x + K * 0.5, p - 0.5 * K, p + 0.5 * K);
    return vec3(0.5 * (omin + omax), (omax.x - omin.x) * (omax.y - omin.y) / (K.x * K.y));
}

vec4 FluidData(particle P, vec2 pos) {
    float den = 0.0;
    vec3 curl = vec3(0.0);

    vec2 gradki = vec2(0.0);
    float gradl = 0.0;

    float rho = (P.M < 1.0) ? fluid_rho * 0.5 : fluid_rho * P.M;

    int I = int(ceil(particle_rad * pixel_scale));
    range(i, -I, I) range(j, -I, I) {
        vec2 tpos = pos + vec2(i, j);
        vec4 data = texel(ch0, tpos);
        particle P0 = getParticle(data, tpos);

        if(P0.M == 0.0)
            continue;

        //density
        den += W(P.NX - P0.NX, particle_rad) * P0.M;
        //den += distribution(P.NX, P0.NX, vec2(particle_rad*pixel_scale)).z * P0.M;

        //gradient
        vec2 g = WS(P.NX - P0.NX, particle_rad) * P0.M / rho;// * particle_rad*pixel_scale;
        gradki += g;
        gradl += dot2(g);

        //curl
        if(i == 0 && j == 0)
            continue;
        vec2 u = (vel(P) - vel(P0)) * P0.M;
        vec2 v = WS(P.NX - P0.NX, particle_rad);
        //vec2 v = W(P.NX - P0.NX, particle_rad) * normalize(P.NX - P0.NX);
        curl += cross(vec3(u, 0.0), vec3(v, 0.0));
    }
    gradl += dot2(gradki);

    //pressure
    float Y = 3.0;
    float C = 0.08;
    float pr = ((fluid_rho * C) / Y) * (pow(den / rho, Y) - 1.0);

    //pr = den/fluid_rho - 1.0;

    //some hardcoded stuff
    //gas
    if(P.M < 1.0)
        pr = den * 0.02;
    //pr = 0.02*(den-rho);

    //pr = max(pr, 0.0);
    if(pr < 0.0)
        pr *= 0.1;

    float l = pr / (gradl + 0.01);

    return vec4(den, pr, l, curl.z);
}

void mainImage(out vec4 O, in vec2 pos) {
    R = iResolution.xy;
    time = iTime;
    ivec2 p = ivec2(pos);

    vec4 data = texel(ch0, pos);
    particle P = getParticle(data, pos);

    if(P.M > 0.0)
        O = FluidData(P, pos);
}

void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}