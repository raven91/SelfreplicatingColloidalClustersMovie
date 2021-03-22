#version 330

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

in GeometryShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
    vec3 color;
} gs_out;

out vec4 color_out_of_fragment_shader;

//uniform float colloid_diameter;
//uniform mat4 particle_translation;
//uniform vec3 system_size;
//uniform vec3 light_pos;
uniform vec3 view_pos;
//uniform vec3 object_color;

void main()
{
    /*mat4 scale_by_radius = mat4(1.0f);
    scale_by_radius[0][0] = 0.5f * colloid_diameter;
    scale_by_radius[1][1] = 0.5f * colloid_diameter;
    scale_by_radius[2][2] = 0.5f * colloid_diameter;
    vec3 fragment_position = vec3(particle_translation * scale_by_radius * vec4(gs_out.fragment_position, 1.0f));
    if ((fragment_position.x < 0.0f) || (fragment_position.y < 0.0f) || (fragment_position.z < 0.0f)
        || (fragment_position.x > system_size.x) || (fragment_position.y > system_size.y) || (fragment_position.z > system_size.z))
    {
        //        color_out_of_fragment_shader = vec4(1.0f, 0.0f, 0.0f, 0.9f);
        discard;
    } else*/
    {
        float ambient_strength = 1.0f;//0.6f;
        vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
        vec3 ambient_color = light.ambient * material.ambient;//vec3(0.2f, 0.2f, 0.2f);
        vec3 ambient = ambient_strength * ambient_color;

        float diffuse_strength = 1.0f;//0.8f;
        vec3 diffuse_color = light.diffuse * material.diffuse;//vec3(0.5f, 0.5f, 0.5f);
        vec3 normalized_normal = normalize(gs_out.normal);
        vec3 light_direction = normalize(light.position - gs_out.fragment_position_in_world_coordinates);
        float difference = max(dot(light_direction, normalized_normal), 0.0f);
        vec3 diffuse = diffuse_strength * difference * diffuse_color;

        float specular_strength = 1.0f;//0.1f;
        vec3 specular_color = light.specular * material.specular;//vec3(1.0f, 1.0f, 1.0f);
        vec3 view_direction = normalize(view_pos - gs_out.fragment_position_in_world_coordinates);
        vec3 reflect_direction = reflect(-light_direction, normalized_normal);
        float shininess = 8.0f;
        float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess);
        vec3 specular = specular_strength * spec * specular_color;

        //        vec3 object_color = vec3(0.7f, 0.2f, 0.2f);
        vec3 resulting_color = (ambient + diffuse + specular);// * object_color;
        color_out_of_fragment_shader = vec4(resulting_color, 1.0f);
    }
}
