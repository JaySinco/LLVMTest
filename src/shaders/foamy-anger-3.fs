#version 430

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

#define PI 3.14159265
#define hh 1.
#define ch0 iChannel2
#define ch1 iChannel1
#define Bi(p) ivec2(p)
#define pixel_scale 4.0
#define particle_rad 1.0
#define relax_value 1.0 / 2.0
#define fluid_rho 2.5
#define texel(a, p) texelFetch(a, Bi(p), 0)
#define range(i,a,b) for(int i = a; i <= b; i++)

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

mat2 Rot(float ang) {
    return mat2(cos(ang), -sin(ang), sin(ang), cos(ang));
}

float sdBox(in vec2 p, in vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float border(vec2 p) {
    float bound = -sdBox(p - R * 0.5, R * vec2(0.5, 0.5));
    float box = sdBox(Rot(0. * time - 0.0) * (p - R * vec2(0.5, 0.6)), R * vec2(0.05, 0.0125));
    float drain = -sdBox(p - R * vec2(0.5, 0.7), R * vec2(1.5, 0.5));
    //return bound - 10.0;
    return min(bound - 10.0, box);
    return max(drain, min(bound - 10.0, box));
}

bool imBorder(vec2 pos) {
    return border(pos) < 0.0 && mod(pos.x, 1.8) < 1.0 && mod(pos.y, 1.8) < 1.0;
}

vec2 WS(vec2 r, float h) {
    return (length(r / pixel_scale) > 0.0 && h >= length(r / pixel_scale)) ? -(30.0 / (PI * pow(h, 5.0))) * pow(h - length(r / pixel_scale), 2.0) * normalize(r) : vec2(0.0);
}

vec3 bN(vec2 p) {
    vec3 dx = vec3(-hh, 0, hh);
    vec4 idx = vec4(-1. / hh, 0., 1. / hh, 0.25);
    vec3 r = idx.zyw * border(p + dx.zy) + idx.xyw * border(p + dx.xy) + idx.yzw * border(p + dx.yz) + idx.yxw * border(p + dx.yx);
    return vec3(normalize(r.xy), r.z + 1e-4);
}

void Simulation(sampler2D ch, sampler2D chd, inout particle P, vec2 pos) {
    vec2 F = vec2(0.0);
    vec3 n = vec3(0.0);

    vec4 pr = texel(chd, pos);

    //int I = int(ceil(particle_size))+2;
    int I = int(ceil(particle_rad * pixel_scale));
    range(i, -I, I) range(j, -I, I) {
        if(i == 0 && j == 0)
            continue;

        vec2 tpos = pos + vec2(i, j);
        vec4 data = texel(ch, tpos);
        particle P0 = getParticle(data, tpos);

        if(P0.M == 0.0 || tpos.x < 0.0 || tpos.y < 0.0)
            continue;
        if(length(P0.NX - P.NX) > particle_rad * pixel_scale)
            continue;

        vec2 dx = P.NX - P0.NX;
        float d = length(dx);
        float r = P.R + P0.R;
        float m = (((P.M - P0.M) / (P.M + P0.M)) + ((2.0 * P0.M) / (P.M + P0.M)));
        //m = P0.M / (P.M + P0.M);

        float rho = (P.M < 1.0) ? fluid_rho * 0.5 : fluid_rho * P.M;

        vec4 pr0 = texel(chd, tpos);
        float pf = (pr.z + pr0.z);

        //collision
        F += normalize(dx) * max(r - d, 0.0) * m;
        //fluid
        F -= WS(dx, particle_rad) * pf / rho * P0.M;

        //cohesion
        //vec2 co = 0.2 * WC(dx, particle_rad*2.0) * normalize(dx);
        //F -= ((fluid_rho*2.0)/(pr.x+pr0.x))*co;

        //adhesion
        //if (imBorder(tpos))
            //F -= 1.0 * WA(dx, particle_rad) * normalize(dx) * P0.M;

        //curl
        n -= vec3(WS(dx, particle_rad) * abs(pr0.w) * P0.M, 0.0);

        //viscosity
        //F -= 0.01 * WTest(dx, 4.0) * (vel(P) - vel(P0)) * P0.M * (imBorder(tpos) ? 100.0*0.0 : 0.5);
    }

    if(length(n) > 0.0 && pr.z > 0.0)
        F += cross(normalize(n), vec3(0.0, 0.0, pr.w)).xy * 0.1 * pr.z;

    //border
    vec2 dp = P.NX;
    float d = border(dp) - P.R;
    if(d < 0.0)
        F -= bN(dp).xy * d;

    P.NX += F * relax_value;
}

float encode(vec2 x) {
    //uint X = packSnorm2x16(x);
    uint X = packHalf2x16(x);

    return uintBitsToFloat(X);
}

vec4 saveParticle(particle P, vec2 pos) {
    P.X = P.X - pos;
    P.NX = P.NX - pos;
    return vec4(encode(P.X), encode(P.NX), (P.R), (P.M));
}

void mainImage(out vec4 O, in vec2 pos) {
    R = iResolution.xy;
    time = iTime;
    Mouse = iMouse;
    vec4 data = texel(ch0, pos);
    particle P = getParticle(data, pos);

    if(P.M > 0.0 && !imBorder(P.NX))
        Simulation(ch0, ch1, P, pos);

    O = saveParticle(P, pos);
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}