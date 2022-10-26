#version 330

out vec4 finalColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 st = fragCoord.xy / iResolution.xy;
    float y = st.x;
    float z = smoothstep(y - 0.02, y, st.y) - smoothstep(y, y + 0.02, st.y);
    fragColor = vec4(vec3(z), 1);
}

void main() {
    mainImage(finalColor, gl_FragCoord.xy);
}