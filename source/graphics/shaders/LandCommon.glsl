
const float PI = 3.14159265359;

// textures
uniform sampler2DArray terrainTex;
uniform sampler2DArray blendTex;
uniform sampler2D normalTex;
uniform sampler2D offsetTex;

// the true offset of values in offsetTex are * scale, + base
uniform float offsetBase;
uniform float offsetScale;

// lighting parameters
uniform vec4 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float shininess;

uniform mat3 normalMatrix;

// model space to world space
uniform mat4 modelMatrix;

// model space to view space
uniform mat4 modelViewMatrix;

// model space to clip space
uniform mat4 modelViewProjectionMatrix;
