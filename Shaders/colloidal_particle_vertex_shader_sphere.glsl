#version 330

in vec3 position;
in vec3 normal;

out VertexShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out;

uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

void main()
{
    gl_Position = vec4(position.xyz, 1.0f);
    vs_out.fragment_position = position;
    vs_out.fragment_position_in_world_coordinates = vec3(model * vec4(position, 1.0f));
//    vs_out.normal = mat3(transpose(inverse(model))) * normal;
    vs_out.normal = mat3(model) * normal;
}
