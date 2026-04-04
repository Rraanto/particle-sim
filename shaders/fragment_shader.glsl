#version 460 core

out vec4 FragColor; // color output 
flat in int vClass;
in vec3 vColor;

// uniforms 
uniform vec2 uCenter;
uniform float uScale; 
uniform float uAspect; // aspect ratio 
uniform float uTime; 

void main() {
  vec2 coord = gl_PointCoord * 2.0 - 1.0;
  float dist_sq = dot(coord, coord);
  if (dist_sq > 1.0) {
    discard;
  }

  FragColor = vec4(vColor, 1.0);
}
