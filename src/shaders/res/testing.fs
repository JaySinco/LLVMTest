#version 430

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}