#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VertexShaderOut
{
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 system_size;

void main()
{
    gl_Position = projection * view * model * vec4(position.x, position.z, system_size.y-position.y, 1.0f);
    vs_out.fragment_position_in_world_coordinates = vec3(model * vec4(position.x, position.z, system_size.y-position.y, 1.0f));
//        vs_out.normal = mat3(transpose(inverse(model))) * vec3(-normal.x, -normal.z, -normal.y);
    vs_out.normal = mat3(model) * vec3(normal.x, normal.z, normal.y);
}
