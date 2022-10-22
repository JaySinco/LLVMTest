#version 330

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 st = fragCoord.xy / iResolution.xy;
    fragColor = vec4(st.x, st.y, 0.0, 1.0);
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}