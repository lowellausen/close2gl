#version 400 core


layout (location = 0) in vec4 model_coefficients;

layout (location = 1) in vec2 vertex_data;
layout (location = 2) in vec2 texture_coords;

out vec2 texcoords;

subroutine void vertex_close();

subroutine(vertex_close) void no_raster()
{
    gl_Position = vec4(model_coefficients.x, model_coefficients.y, 0.0, 1.0);
}


subroutine(vertex_close) void with_raster()
{
    gl_Position = vec4(vertex_data.x, vertex_data.y, 0.0, 1.0);


    texcoords = texture_coords;
}

subroutine uniform vertex_close vertex_willraster;


void main()
{
    //gl_Position = vec4(model_coefficients.x, model_coefficients.y, 0.0, 1.0);
    //gl_Position = vec4(vertex_data.x, vertex_data.y, 0.0, 1.0);


    //texcoords = texture_coords;


    vertex_willraster();
}
