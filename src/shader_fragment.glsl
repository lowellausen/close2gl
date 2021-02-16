#version 400 core

uniform vec3 in_color;

uniform sampler2D Texture;

in vec2 texcoords;


out vec3 color;

subroutine void frag_close();

subroutine(frag_close) void no_text()
{
    color = in_color;

    //color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

subroutine(frag_close) void with_text()
{
    color = texture(Texture, vec2(texcoords.x,1.0f-texcoords.y)).rgb;
    //color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

subroutine uniform frag_close frag_willtext;


void main()
{

    /*color = in_color;

    color = pow(color, vec3(1.0,1.0,1.0)/2.2);*/

    //color = texture(Texture, vec2(texcoords.x,texcoords.y)).rgb;
    //color = vec3(1.0,0.0,1.0);

    frag_willtext();
}
