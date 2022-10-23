#version 330

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel0;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 data = texelFetch(iChannel0, ivec2(fragCoord), 0);
    // fragColor = vec4(sqrt(data.rgb / data.w), 1.0);
    fragColor = vec4(sqrt(data.rgb), 1.0);
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}