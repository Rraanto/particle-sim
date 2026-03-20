#version 460 core
layout (location = 0) in vec2 aPos;
out vec2 vPos; // export to be usasble in the fragment

uniform vec2 uCenter;
uniform float uScale;
uniform float uZoom;
uniform float uAspect;

void main()
{
  vec2 pos = (aPos - uCenter) / (uScale * uZoom);
  pos.x /= uAspect;
  gl_Position = vec4(pos, 0.0, 1.0);
  vPos = aPos;
}
