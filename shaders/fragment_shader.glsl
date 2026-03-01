#version 460 core

out vec4 FragColor; // color output 
in vec2 vPos; // read from vertex shader output

// uniforms 
uniform vec2 uCenter;
uniform float uScale; 
uniform float uAspect; // aspect ratio 
uniform float uTime; 

void main() {
  float scale = uScale; 
  vec2 position = vPos; 

  FragColor = vec4(1.0, 1.0, 1.0, 1.0); // just white
}

