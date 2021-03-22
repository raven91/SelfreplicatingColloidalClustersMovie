#version 330

flat in int particle_type;
in GeometryShaderOut
{
    vec3 uv;
    vec3 color;
} fs_in;

out vec4 color_out_of_fragment_shader;

void main()
{
    vec3 dist_vec = fs_in.uv - vec3(0.5f, 0.5f, 0.0f);
    float dist_squared = dot(dist_vec, dist_vec);
    float clipping_radius = 0.5f;

    if (dist_squared > clipping_radius * clipping_radius)
    {
        discard;
    }
    else
    {
        float ambientStrength = 1.0f;
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        vec3 ambient = ambientStrength * lightColor;
//        color_out_of_fragment_shader = vec4(ambient * vec3(0.0f, 0.0f, 0.0f), 0.5f);
        color_out_of_fragment_shader = vec4(ambient * fs_in.color, 1.0f);
    }
}
