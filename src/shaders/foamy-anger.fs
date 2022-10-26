#version 430

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel1;
uniform sampler2D iChannel3;

#define ch0 iChannel3
#define ch1 iChannel1
#define particle_size 1.6
#define Bi(p) ivec2(p)
#define saturate(x) clamp(x, 0.0, 1.0)
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

float sminMe(float a, float b, float k, float p, out float t) {
    float h = max(k - abs(a - b), 0.0) / k;
    float m = 0.5 * pow(h, p);
    t = (a < b) ? m : 1.0 - m;
    return min(a, b) - (m * k / p);
}

float smin(in float a, in float b, in float k) {
    float h = max(k - abs(a - b), 0.0);
    float m = 0.25 * h * h / k;
    return min(a, b) - m;
}

void mainImage(out vec4 O, in vec2 pos) {
    R = iResolution.xy;
    time = iTime;

    O.xyz = vec3(1.0);

    float d = 100.0;

    vec3 c = vec3(1.0);
    float m = 1.0;
    float v = 0.0;

    //rendering
    int I = int(ceil(particle_size * 0.5)) + 2;
    range(i, -I, I) range(j, -I, I) {
        vec2 tpos = pos + vec2(i, j);
        vec4 data = texel(ch0, tpos);
        particle P0 = getParticle(data, tpos);

        if(P0.M == 0.0)
            continue;

        float nd = distance(pos, P0.NX) - P0.R;

        float k = 4.0 / (1.0 + abs(m - P0.M) * 1.5);
        float t;
        d = sminMe(d, nd, k, 3.0, t);
        m = mix(m, P0.M, t);
        v = mix(v, texel(ch1, tpos).w, t);
    }

    //shadow
    float s = 100.0;
    vec2 off = vec2(10.0, 20.0);
    if(d > 0.0) {
        range(i, -I, I) range(j, -I, I) {
            vec2 tpos = pos - off + vec2(i, j);
            vec4 data = texel(ch0, tpos);
            particle P0 = getParticle(data, tpos);

            if(tpos.x < 0.0 || tpos.x > R.x || tpos.y < 0.0 || tpos.y > R.x) {
                s = 0.0;
                break;
            }
            if(P0.M == 0.0) {
                continue;
            }

            float nd = distance(pos - off, P0.NX) - P0.R;
            if(texel(ch1, tpos).x > 1.0)
                s = smin(s, nd, 3.0);
        }
    }

    //coloring and stuff
    if(d < 0.0)
        //d = 1.0-cos(d);
        d = sin(d);
    O.xyz *= vec3(abs(d));
    if(d < 0.0) {
        O.xyz *= c;
        O.xyz /= m * 2.0;
        //col.xyz /= 0.5 + m*0.25;
        O.xyz -= vec3(v) / m * 0.06;
    }

    //checkerboard
    if(d > 1.0) {
        float size = 100.0;
        float cy = step(mod(pos.y, size), size * 0.5);
        float ct = step(mod(pos.x + cy * size * 0.5, size), size * 0.5);

        ct = saturate(ct + 0.0);
        //col.xyz = mix(vec3(ct), col.xyz, 1.0-saturate(d));
    }

    O.xyz = saturate(O.xyz);
    if(d > 0.0)
        O.xyz *= mix(vec3(0.7), vec3(1.0), saturate(s));

    O = vec4(pow(O.xyz, vec3(0.7)), 1.0);
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}