#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

#define ONE_PIXEL (1.0/iResolution.y)
#define SMOOTH_PIXEL (2.0 * ONE_PIXEL)

vec2 st;

void draw(float p, vec3 color) {
    finalColor = vec4(mix(finalColor.rgb, color.rgb, clamp(p, 0.0, 1.0)), 1.0);
}

float solidRect(vec2 bottomLeft, vec2 size) {
    vec2 bl = step(vec2(bottomLeft.x, bottomLeft.y), st);
    vec2 tr = 1.0 - step(vec2(bottomLeft.x + size.x, bottomLeft.y + size.y), st);
    return bl.x * bl.y * tr.x * tr.y;
}

float solidCircle(vec2 center, float radius) {
    float p = 1 - smoothstep(radius - SMOOTH_PIXEL * 0.5, radius + SMOOTH_PIXEL * 0.5, distance(st, center));
    return p;
}

float solidLine(vec2 p1, vec2 p2, float lw) {
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
    float l1 = solidCircle(p1, lw * 0.5);
    float l2 = solidCircle(p2, lw * 0.5);
    return p > 0 ? p : (l1 > 0 ? l1 : l2);
}

void drawLine(vec2 p1, vec2 p2, vec3 color, float lw) {
    float p = solidLine(p1, p2, lw);
    draw(p, color);
}

void drawSolidRect(vec2 bottomLeft, vec2 size, vec3 color) {
    float p = solidRect(bottomLeft, size);
    draw(p, color);
}

void drawRect(vec2 bottomLeft, vec2 size, vec3 color, float lw) {
    float p1 = solidRect(bottomLeft - vec2(lw * 0.5), size + vec2(lw * 1.0));
    float p2 = solidRect(bottomLeft + vec2(lw * 0.5), size - vec2(lw * 1.0));
    draw(p1 * (1 - p2), color);
}

void drawSolidCircle(vec2 center, float radius, vec3 color) {
    float p = solidCircle(center, radius);
    draw(p, color);
}

void drawCircle(vec2 center, float radius, vec3 color, float lw) {
    float p1 = solidCircle(center, radius + lw * 0.5);
    float p2 = solidCircle(center, radius - lw * 0.5);
    draw(p1 * (1 - p2), color);
}

void main() {
    st = gl_FragCoord.xy / iResolution.y;
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    float lineWidth = 5.0 * ONE_PIXEL;

    drawRect(vec2(0.5, 0.5), vec2(0.4, 0.4), vec3(0.1, 0.8, 0.1), lineWidth);

    drawSolidCircle(vec2(0.7, 0.25), 0.2, vec3(0.93f, 0.56f, 0.02f));
    drawCircle(vec2(0.7, 0.25), 0.2, vec3(0.85f, 0.84f, 0.03f), lineWidth);

    drawLine(vec2(0.2, 0.9), vec2(1.2, 0.2), vec3(0.6, 0.2, 0.8), lineWidth);
    drawLine(vec2(0.2, 0.9), vec2(1.2, 0.7), vec3(0.6, 0.2, 0.8), lineWidth);

    drawSolidRect(vec2(0.95, 0.45), vec2(0.2, 0.1), vec3(0.07f, 0.36f, 0.34f));
    drawRect(vec2(0.95, 0.45), vec2(0.2, 0.1), vec3(0.8, 0.2, 0.1), lineWidth);
}