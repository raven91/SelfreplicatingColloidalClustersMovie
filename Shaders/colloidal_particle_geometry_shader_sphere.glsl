#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out[3];

out GeometryShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
    vec3 color;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float colloid_diameter;
uniform mat4 particle_translation;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    mat4 scale_by_radius = mat4(1.0f);
    scale_by_radius[0][0] = 0.5f * colloid_diameter;
    scale_by_radius[1][1] = 0.5f * colloid_diameter;
    scale_by_radius[2][2] = 0.5f * colloid_diameter;
    mat4 mvp = projection * view * model * particle_translation * scale_by_radius;
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    gs_out.color = vec3(rand(p1.xy), rand(p1.xy), rand(p1.xy));

    gs_out.normal = vs_out[0].normal;
    gs_out.fragment_position = vs_out[0].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[0].fragment_position_in_world_coordinates;
    gl_Position = mvp * vec4(p0, 1.0f); EmitVertex();

    gs_out.normal = vs_out[1].normal;
    gs_out.fragment_position = vs_out[1].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[1].fragment_position_in_world_coordinates;
    gl_Position = mvp * vec4(p1, 1.0f); EmitVertex();

    gs_out.normal = vs_out[2].normal;
    gs_out.fragment_position = vs_out[2].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[2].fragment_position_in_world_coordinates;
    gl_Position = mvp * vec4(p2, 1.0f); EmitVertex();
    EndPrimitive();
}
