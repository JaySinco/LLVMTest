#version 330

in vec3 vertexPosition;

void main() {
    gl_Position = vec4(vertexPosition.xyz, 1.0);
}