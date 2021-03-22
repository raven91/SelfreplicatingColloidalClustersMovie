#version 330

in vec3 position;
in int particle_type;

out VertexShaderOut
{
    int particle_type;
} vs_out;

void main()
{
    gl_Position = vec4(position.xyz, 1.0f);
//    gl_PointSize = 1.0;

    vs_out.particle_type = particle_type;
}
