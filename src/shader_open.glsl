#version 400 core


layout (location = 0) in vec3 model_coefficients;
layout (location = 1) in vec3 normal_coefficients;

layout (location = 2) in vec2 texture_coordinates;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 in_color;

uniform int shading_index;

out vec4 position_world;
out vec4 position_model;
out vec4 normal;

out vec3 out_color;

out vec2 text_coord;



subroutine void vertex_type();

subroutine(vertex_type) void none()
{

    out_color = in_color;
}

subroutine(vertex_type) void gourAD()
{
    vec3 Kd = in_color;
    vec3 Ks = in_color;
    vec3 Ka = in_color;
    vec3 light_spectrum = vec3(1.0, 1.0, 1.0);
    vec3 ambient_light_spectrum = vec3(0.2, 0.2, 0.2);

    vec4 camera_position = inverse(view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 p = position_world;

    vec4 light_src_position = inverse(view) * vec4(2.0f, 2.0f, 2.0f, 1.0f);
    vec4 l = normalize(light_src_position - p);


    float diffuse = max( 0 , dot(normal, l));

    out_color = ( Ka * ambient_light_spectrum + Kd * light_spectrum * diffuse);
}

subroutine(vertex_type) void gourADS()
{
    vec3 Kd = in_color;
    vec3 Ks = in_color;
    vec3 Ka = in_color;
    float q = 5.0f;

    vec3 light_spectrum = vec3(1.0,1.0,1.0);
    vec3 ambient_light_spectrum = vec3(0.2,0.2,0.2);

    vec4 camera_position = inverse(view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 p = position_world;

   vec4 light_src_position = inverse(view) * vec4(2.0f, 2.0f, 2.0f, 1.0f);
    vec4 l = normalize(light_src_position - p);

    vec4 v = normalize(camera_position - p);

    vec4 r = -l + 2 * normal * dot(normal, l);
    float diffuse = max( 0 , dot(normal, l));
    float specular  = pow( max( 0, dot(r, v)), q);


    out_color = ( Ka * ambient_light_spectrum + Kd * light_spectrum * diffuse + Ks * light_spectrum * specular);
}

subroutine(vertex_type) void phong()
{
    //position_world = model * model_coefficients;

    //normal = inverse(transpose(view*model)) * normal_coefficients;
    //normal = normal_coefficients;
    //normal.w = 0.0;
    int opa=0;

    //out_color = vec3(0.5f, 0.0f, 0.5f);
}

subroutine uniform vertex_type vertex_shading_type;



void main()
{
    //normal_coefficients = vec4(normal_coefficients3.x,normal_coefficients3.y,normal_coefficients3.z,0.0f);
    //vec4 model_coefficients4 =  vec4(model_coefficients.x,model_coefficients.y,model_coefficients.z, 1.0f);
    //por algum motivo pode receber vec4 aqui e passar vec3 na mainq ue tá de boa
        // posição do vertice em coordenadas do modelo
    position_model = vec4(model_coefficients, 1.0);
    position_world = model * position_model; //posição do vertice em coordenadas globais
    gl_Position = projection * view * model * position_model; // posição final do vertice na projeção

    normal = inverse(transpose(model)) * vec4(normal_coefficients, 0.0); // normal do vertice em coord globais
    normal.w = 0.0;

    text_coord = texture_coordinates; //acho que não faz mal chamar sempre


    vertex_shading_type();

}

