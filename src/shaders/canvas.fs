#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

#define PI 3.14159265
#define COORD_SCALE 16.0
#define ONE_PIXEL (COORD_SCALE / iResolution.y)
#define SMOOTH_PIXEL (3.0 * ONE_PIXEL)
#define d2a(d) (d / 180.0 * PI)

vec2 translate(vec2 st, vec2 d) {
    return st - d;
}

vec2 rotate(vec2 st, float a) {
    return mat2(cos(-a), -sin(-a), sin(-a), cos(-a)) * st;
}

vec2 scale(vec2 st, vec2 c) {
    return mat2(1.0 / c.x, 0.0, 0.0, 1.0 / c.y) * st;
}

void draw(float p, vec3 color) {
    finalColor = vec4(mix(finalColor.rgb, color.rgb, clamp(p, 0.0, 1.0)), 1.0);
}

float solidRect(vec2 uv, vec2 size) {
    vec2 bl = smoothstep(size * -0.5 - SMOOTH_PIXEL * 0.5, size * -0.5 + SMOOTH_PIXEL * 0.5, uv);
    vec2 tr = 1.0 - smoothstep(size * 0.5 - SMOOTH_PIXEL * 0.5, size * 0.5 + SMOOTH_PIXEL * 0.5, uv);
    return bl.x * bl.y * tr.x * tr.y;
}

float wireRect(vec2 uv, vec2 size, float lw) {
    float p1 = solidRect(uv, size + vec2(lw * 1.0));
    float p2 = solidRect(uv, size - vec2(lw * 1.0));
    return p1 * (1 - p2);
}

float solidCircle(vec2 uv, float radius) {
    float p = 1 - smoothstep(radius - SMOOTH_PIXEL * 0.5, radius + SMOOTH_PIXEL * 0.5, length(uv));
    return p;
}

float wireCircle(vec2 uv, float radius, float lw) {
    float p1 = solidCircle(uv, radius + lw * 0.5);
    float p2 = solidCircle(uv, radius - lw * 0.5);
    return p1 * (1 - p2);
}

float line(vec2 st, vec2 p1, vec2 p2, float lw) {
    float a = distance(p1, st);
    float b = distance(p2, st);
    float c = distance(p1, p2);
    float p = 0.0;
    if(a > c || b > c) {
        p = 0.0;
    } else {
        float p0 = (a + b + c) * 0.5;
        float h = 2 / c * sqrt(p0 * (p0 - a) * (p0 - b) * (p0 - c));
        p = 1.0 - smoothstep(0.5 * lw - 0.5 * SMOOTH_PIXEL, 0.5 * lw + 0.5 * SMOOTH_PIXEL, h);
    }
    float l1 = solidCircle(translate(st, p1), lw * 0.5);
    float l2 = solidCircle(translate(st, p2), lw * 0.5);
    return p > 0 ? p : (l1 > 0 ? l1 : l2);
}

void drawGrid(vec2 st, float lw) {
    vec2 uv = fract(st);
    float grid = smoothstep(0, lw, 0.5 - max(abs(uv.x - 0.5), abs(uv.y - 0.5)));
    float axis = smoothstep(lw * 1.0, lw * 2.0, min(abs(st.x), abs(st.y)));
    finalColor = vec4(vec3(min(grid, axis) * 0.25 + 0.75), 0.0);
}

void main() {
    vec2 st = (gl_FragCoord.xy - iResolution.xy / 2.0) * ONE_PIXEL;
    float lw = 5.0 * ONE_PIXEL;

    drawGrid(st, 1 * ONE_PIXEL);

    draw(wireRect(translate(rotate(st, d2a(85.0)), vec2(3.0, 5.0)), vec2(4.0, 4.0), lw), vec3(0.1, 0.8, 0.1));

    draw(solidCircle(translate(st, vec2(-7, -4)), 3), vec3(0.93, 0.56, 0.02));
    draw(wireCircle(translate(st, vec2(-7, -4)), 3, lw), vec3(0.85, 0.84, 0.03));

    draw(line(st, vec2(7.0, 5.0), vec2(2.0, -5.0), lw), vec3(0.6, 0.2, 0.8));
    draw(line(st, vec2(7.0, 5.0), vec2(-8.0, 3.0), lw), vec3(0.6, 0.2, 0.8));
    draw(line(st, vec2(2.0, -5.0), vec2(-8.0, 3.0), lw), vec3(0.6, 0.2, 0.8));

    draw(solidRect(translate(rotate(st, d2a(18)), vec2(-2.0, 1.0)), vec2(5, 2)), vec3(0.07f, 0.36f, 0.34f));
    draw(wireRect(translate(rotate(st, d2a(18)), vec2(-2.0, 1.0)), vec2(5, 2), lw), vec3(0.8, 0.2, 0.1));
}