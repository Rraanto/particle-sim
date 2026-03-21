#version 460 core

out vec4 FragColor; // color output 
in vec2 vPos; // read from vertex shader output
flat in int vClass;

// uniforms 
uniform vec2 uCenter;
uniform float uScale; 
uniform float uAspect; // aspect ratio 
uniform float uTime; 

void main() {
  float scale = uScale; 
  vec2 position = vPos; 

  vec3 palette[8] = vec3[8](
    vec3(0.90, 0.25, 0.25), // red
    vec3(0.25, 0.80, 0.35), // green
    vec3(0.20, 0.45, 0.90), // blue
    vec3(0.95, 0.85, 0.30), // yellow
    vec3(0.80, 0.35, 0.80), // magenta
    vec3(0.20, 0.85, 0.85), // cyan
    vec3(0.95, 0.55, 0.20), // orange
    vec3(0.60, 0.60, 0.60)  // gray
  );
  int idx = abs(vClass) % 8;
  vec3 color = palette[idx];
  FragColor = vec4(color, 1.0);
}
