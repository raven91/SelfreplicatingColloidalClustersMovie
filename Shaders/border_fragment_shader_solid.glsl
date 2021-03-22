#version 330

in VertexShaderOut
{
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out;

out vec4 out_color;

uniform vec3 light_pos;
uniform vec3 view_pos;

void main()
{
    float ambient_strength = 0.6f;
    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
    vec3 ambient = ambient_strength * light_color;

    float diffuse_strength = 0.8f;
    vec3 normalized_normal = normalize(vs_out.normal);
    vec3 light_direction = normalize(light_pos - vs_out.fragment_position_in_world_coordinates);
    float difference = max(dot(light_direction, normalized_normal), 0.0f);
    vec3 diffuse = diffuse_strength * difference * light_color;

    float specular_strength = 0.1f;
    vec3 view_direction = normalize(view_pos - vs_out.fragment_position_in_world_coordinates);
    vec3 reflect_direction = reflect(-light_direction, normalized_normal);
    float shininess = 8.0f;
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), shininess);
    vec3 specular = specular_strength * spec * light_color;

    vec3 object_color = vec3(0.9f, 0.9f, 0.9f);
    vec3 resulting_color = (ambient + diffuse + specular) * object_color;
    out_color = vec4(resulting_color, 0.3f);
}
