#version 330 core

layout(location = 0) in vec2 position;
uniform mat4 VP;
void main() {
  gl_Position = VP * vec4(position, 0.0, 1.0);
}