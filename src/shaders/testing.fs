#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

float onePix(float px) {
    return px / iResolution.y;
}

float drawSolidRect(vec2 st, vec2 bottomLeft, vec2 size) {
    vec2 bl = step(vec2(bottomLeft.x, bottomLeft.y), st);
    vec2 tr = step(vec2(1.0 - bottomLeft.x - size.x, 1.0 - bottomLeft.y - size.y), 1.0 - st);
    return bl.x * bl.y * tr.x * tr.y;
}

float drawSolidCircle(vec2 st, vec2 center, float radius) {
    return 1 - step(radius, distance(st, center));
}

float drawLineAdv(vec2 st, vec2 p1, vec2 p2, float px, bool do_smooth) {
    float weight = onePix(px);
    float d = distance(p1, p2);
    vec2 p1c = p1;
    vec2 p2c = p2;
    p1.x = p1c.x + (p1c.x - p2c.x) / d * weight * 0.8;
    p1.y = p1c.y + (p1c.y - p2c.y) / d * weight * 0.8;
    p2.x = p2c.x + (p2c.x - p1c.x) / d * weight * 0.8;
    p2.y = p2c.y + (p2c.y - p1c.y) / d * weight * 0.8;

    float a = distance(p1, st);
    float b = distance(p2, st);
    float c = distance(p1, p2);

    if(a > c || b > c)
        return 0.0;

    float p = (a + b + c) * 0.5;
    float h = 2 / c * sqrt(p * (p - a) * (p - b) * (p - c));
    if(do_smooth) {
        return mix(1.0, 0.0, smoothstep(0.5 * weight, 1.5 * weight, h));
    } else {
        return 1.0 - step(0.8 * weight, h);
    }
}

float drawLine(vec2 st, vec2 p1, vec2 p2, float px) {
    return drawLineAdv(st, p1, p2, px, true);
}

float drawRect(vec2 st, vec2 bottomLeft, vec2 size, float px) {
    float bottom = drawLineAdv(st, bottomLeft, bottomLeft + vec2(size.x, 0.0), px, false);
    float top = drawLineAdv(st, bottomLeft + vec2(0.0, size.y), bottomLeft + vec2(size.x, size.y), px, false);
    float left = drawLineAdv(st, bottomLeft, bottomLeft + vec2(0.0, size.y), px, false);
    float right = drawLineAdv(st, bottomLeft + vec2(size.x, 0.0), bottomLeft + vec2(size.x, size.y), px, false);
    return clamp(bottom + top + left + right, 0.0, 1.0);
}

float drawCircle(vec2 st, vec2 center, float radius, float px) {
    float weight = onePix(px);
    float h = abs(radius - distance(st, center));
    return mix(1.0, 0.0, smoothstep(0.5 * weight, 1.5 * weight, h));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 st = fragCoord.xy / iResolution.y;
    vec3 color = vec3(0.0);

    float t1 = drawRect(st, vec2(0.5, 0.5), vec2(0.4, 0.4), 2);
    color += vec3(0.1, 0.8, 0.1) * t1;

    float t2 = drawRect(st, vec2(0.1, 0.3), vec2(0.7, 0.5), 2);
    color += vec3(0.8, 0.2, 0.1) * t2;

    float t3 = drawCircle(st, vec2(0.2, 0.2), 0.3, 2);
    color += vec3(0.1, 0.2, 0.8) * t3;

    float t4 = drawLine(st, vec2(0.6, 0.1), vec2(0.6, 0.7), 2);
    color += vec3(0.6, 0.2, 0.8) * t4;

    float t5 = drawLine(st, vec2(0.2, 0.9), vec2(1.2, 0.7), 2);
    color += vec3(0.6, 0.2, 0.8) * t5;

    fragColor = vec4(color, 1.0);
}

void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}