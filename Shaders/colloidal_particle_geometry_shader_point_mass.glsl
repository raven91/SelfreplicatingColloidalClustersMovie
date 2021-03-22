#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VertexShaderOut
{
    int particle_type;
} gs_in[1];

flat out int particle_type;
out GeometryShaderOut
{
    vec3 uv;
    vec3 color;
} gs_out;

uniform vec3 camera_pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float colloid_diameter;
//uniform mat4 particle_translation;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    mat4 mvp = projection * view * model;// * particle_translation;
    vec3 center = gl_in[0].gl_Position.xyz;
    vec3 direction_to_camera = normalize(vec3(inverse(model) * vec4(camera_pos, 1.0f)) - vec3(vec4(center, 1.0f)));
    vec3 vx = normalize(vec3(1.0f, 1.0f, -direction_to_camera.x / direction_to_camera.z * 1.0f - direction_to_camera.y / direction_to_camera.z * 1.0f));
    vec3 vy = cross(direction_to_camera, vx);

    vec3 A = center - 0.5 * colloid_diameter * vy - 0.5 * colloid_diameter * vx;
    vec3 B = center - 0.5 * colloid_diameter * vy + 0.5 * colloid_diameter * vx;
    vec3 C = center + 0.5 * colloid_diameter * vy + 0.5 * colloid_diameter * vx;
    vec3 D = center + 0.5 * colloid_diameter * vy - 0.5 * colloid_diameter * vx;

    particle_type = gs_in[0].particle_type;
    gs_out.color = vec3(rand(A.xy), rand(B.xy), rand(C.xy));

    gs_out.uv = vec3(1, 1, 0); gl_Position = mvp * vec4(C, 1.0f); EmitVertex();
    gs_out.uv = vec3(0, 1, 0); gl_Position = mvp * vec4(D, 1.0f); EmitVertex();
    gs_out.uv = vec3(1, 0, 0); gl_Position = mvp * vec4(B, 1.0f); EmitVertex();
    gs_out.uv = vec3(0, 0, 0); gl_Position = mvp * vec4(A, 1.0f); EmitVertex();
    EndPrimitive();
}
