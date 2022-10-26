#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel3;

#define ch0 iChannel3
#define dt 1.0
#define UV (pos/R)
#define particle_size 1.6
#define range(i,a,b) for(int i = a; i <= b; i++)
#define Bi(p) ivec2(p)
#define texel(a, p) texelFetch(a, Bi(p), 0)

vec2 R;
uvec4 s0;
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

vec2 vel(particle P) {
    return P.NX - P.X;
}

void Integrate(sampler2D ch, inout particle P, vec2 pos) {
    int I = 2;
    range(i, -I, I) range(j, -I, I) {
        vec2 tpos = pos + vec2(i, j);
        vec4 data = texel(ch, tpos);

        if(tpos.x < 0.0 || tpos.y < 0.0)
            continue;

        particle P0 = getParticle(data, tpos);

        //falls in this pixel
        if(P0.NX.x >= pos.x - 0.5 && P0.NX.x < pos.x + 0.5 && P0.NX.y >= pos.y - 0.5 && P0.NX.y < pos.y + 0.5 && P0.M > 0.0) {
            vec2 P0V = vel(P0) / dt;

            //external forces
            if(iMouse.z > 0.0) {
                vec2 dm = (iMouse.xy - iMouse.zw) / 10.;
                float d = distance(iMouse.xy, P0.NX) / 20.;
                P0V += 0.005 * dm * exp(-d * d) * 1.0;
            }

            P0V += vec2(0., -0.005) * ((P0.M < 0.95) ? 0.05 : 1.0);//*P0.M;
            //P0V -= normalize(P0.NX - iResolution.xy*0.5) * 0.005 * ((P0.M < 0.95) ? 0.05 : 1.0);

            float v = length(P0V);
            P0V /= (v > 1.0) ? v : 1.0;

            //
            P0.X = P0.NX;
            P0.NX = P0.NX + P0V * dt;
            P = P0;
            break;
        }
    }
}

int emitTime(float area, float pc) {
    float ppf = area / particle_size;
    return int(((R.x * R.y) / ppf) * pc);
}

void rng_initialize(vec2 p, int frame) {
    s0 = uvec4(p, uint(frame), uint(p.x) + uint(p.y));
}

float sdBox(in vec2 p, in vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

mat2 Rot(float ang) {
    return mat2(cos(ang), -sin(ang), sin(ang), cos(ang));
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

particle getVP(vec2 pos) {
    particle P;

    P.X = pos;
    P.NX = pos;
    P.M = 1.25;
    P.R = 1.8 * 0.5;
    return P;
}

// https://www.pcg-random.org/
void pcg4d(inout uvec4 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
}

float rand() {
    pcg4d(s0);
    return float(s0.x) / float(0xffffffffu);
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
    time = iTime;
    R = iResolution.xy;
    rng_initialize(pos, iFrame);

    particle P;

    Integrate(ch0, P, pos);

    if(imBorder(pos))
        P = getVP(pos);

    //liquid emitter
    if(P.M == 0.0 && pos.x > 10.0 && pos.x < 11.0 && UV.y > 0.6 && UV.y < 0.75 && mod(pos.y, particle_size * 2.0) < 1.0 && rand() > 0.5 && iFrame < emitTime(R.x * 0.15 * 0.5, 0.18) && true) {
        P.X = pos;
        P.NX = pos + vec2(1.0, 0.0);
        P.M = 1.0;
        P.R = particle_size * 0.5;
    }
    //gas emitter
    if(P.M == 0.0 && pos.x > R.x - 11.0 && pos.x < R.x - 10.0 && UV.y > 0.6 && UV.y < 0.75 && mod(pos.y, particle_size * 2.0) < 1.0 && rand() > 0.25 && iFrame < emitTime(R.x * 0.15 * 0.75, 0.4) && true) {
        P.X = pos;
        P.NX = pos - vec2(0.5, 0.0);
        P.M = 0.5;// + sin(iTime)*0.05;
        P.R = particle_size * 0.5;
    }
    //dense liquid emitter
    if(P.M == 0.0 && pos.x > R.x - 11.0 && pos.x < R.x - 10.0 && UV.y > 0.2 && UV.y < 0.3 && mod(pos.y, particle_size * 2.0) < 1.0 && rand() > 0.25 && iFrame < emitTime(R.x * 0.15 * 0.75, 0.05) && true) {
        P.X = pos;
        P.NX = pos - vec2(0.5, 0.0);
        P.M = 2.5;
        P.R = particle_size * 0.5;
    }

    O = saveParticle(P, pos);
}

void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}