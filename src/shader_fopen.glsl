#version 400 core

in vec4 position_world;
in vec4 normal;
in vec3 out_color;

uniform sampler2D Texture;

in vec2 text_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int shading_index;

uniform vec3 in_color;

out vec3 color;



subroutine vec3 frag_type();

subroutine(frag_type) vec3 nonef()
{
    return out_color;
}

subroutine(frag_type) vec3 textf()
{
    color = texture(Texture, vec2(text_coord.x,text_coord.y)).rgb;
    if(shading_index!=3)return pow(color, vec3(1.0,1.0,1.0)/2.2);

    //se tem iluminação de phong aplicamos ela, se não passamos apenas a cor da textura

    vec3 Kd = in_color;
    vec3 Ks = in_color;
    vec3 Ka = in_color;
    float q = 5.0f;


    vec4 camera_position = inverse(view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 p = position_world;
    vec4 n = normalize(normal);

    vec4 light_src_position = inverse(view) * vec4(2.0f, 2.0f, 2.0f, 1.0f);
    vec4 l = normalize(light_src_position - p);

    vec4 v = normalize(camera_position - p);

    vec4 r = -l + 2 * n * dot(n, l);
    float diffuse = max( 0 , dot(n, l));
    float specular  = pow( max( 0, dot(r, v)), q);



    vec3 light_spectrum = vec3(1.0,1.0,1.0);
    vec3 ambient_light_spectrum = vec3(0.2,0.2,0.2);

    //return   ( Ka * ambient_light_spectrum + Kd * light_spectrum * diffuse + Ks * light_spectrum * specular);


    color = (color * ambient_light_spectrum + color * light_spectrum * diffuse+ Ks*light_spectrum*specular);

    //return color;
    return pow(color, vec3(1.0,1.0,1.0)/2.1);

}

subroutine(frag_type) vec3 phongf()
{


    vec3 Kd = in_color;
    vec3 Ks = in_color;
    vec3 Ka = in_color;
    float q = 5.0f;


    vec4 camera_position = inverse(view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 p = position_world;
    vec4 n = normalize(normal);

    vec4 light_src_position = inverse(view) * vec4(2.0f, 2.0f, 2.0f, 1.0f);
    vec4 l = normalize(light_src_position - p);

    vec4 v = normalize(camera_position - p);

    vec4 r = -l + 2 * n * dot(n, l);
    float diffuse = max( 0 , dot(n, l));
    float specular  = pow( max( 0, dot(r, v)), q);



    vec3 light_spectrum = vec3(1.0,1.0,1.0);
    vec3 ambient_light_spectrum = vec3(0.2,0.2,0.2);

    return   ( Ka * ambient_light_spectrum + Kd * light_spectrum * diffuse + Ks * light_spectrum * specular);

}


subroutine uniform frag_type frag_shading_type;


void main()
{

    color = frag_shading_type();

    //color = pow(color, vec3(1.0,1.0,1.0)/2.2);

}

