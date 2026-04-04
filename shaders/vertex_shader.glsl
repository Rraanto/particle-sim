#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in int aClass;
layout (location = 3) in float aRadius;
flat out int vClass;
out vec3 vColor;

uniform vec2 uCenter;
uniform float uScale;
uniform float uZoom;
uniform float uAspect;

void main()
{
  vec2 pos = (aPos - uCenter) / (uScale * uZoom);
  pos.x /= uAspect;
  gl_Position = vec4(pos, 0.0, 1.0);
  float zoom_scale = max(0.0001, uScale * uZoom);
  gl_PointSize = max(1.0, (2.0 * aRadius) / zoom_scale);
  vClass = aClass;
  vColor = aColor;
}
