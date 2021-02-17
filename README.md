# close2gl

(project done in 2018)  
Close2GL: an implementation of the core functionalities of OpenGL but on CPU using C++. All features are also implemented
on normal OpenGL/GLSL for comparison.

### what is implemented:
All matrix and vector operations, single model (.obj) loading, geometric transforms (rotation, translation), FOV + near and far
planes assignment, free and look-at cameras, back face culling, model coordinates change from model -> world -> eye -> viewport
-> screen (this is actually done rendering a quad with OpenGL and applying my Close2GL image as a texture on it), a depth buffer,
shading (gouraud and phong), points only and wireframe visualization, rasterization, simple texture mapping (Nearest Neighbour,
bilinear, trilinear/mipmap).

A simple user interface was made using [Dear ImGui](https://github.com/ocornut/imgui).

### Screenshots

Options menu:

![Alt text](screenshots/menu.png?raw=true "Menu")

Wireframe rendering:

![Alt text](screenshots/close2wireframeshading.png?raw=true "close2wireframeshading")

Point Rendering:

![Alt text](screenshots/close2pointshading.png?raw=true "close2pointshading")

Solid no shading rendering:

![Alt text](screenshots/close2flat.png?raw=true "close2flat")

Gouraud shading:

![Alt text](screenshots/close2gouraud2.png?raw=true "close2gouraud2")

Phong shading:

![Alt text](screenshots/close2phong.png?raw=true "close2phong")

Testing near and far plane:

![Alt text](screenshots/cutting-planes.png?raw=true "cutting-planes")

Testing FOV control:

![Alt text](screenshots/fovs.png?raw=true "fovs")

Without implementing a z-buffer:

![Alt text](screenshots/nozbuffer.png?raw=true "nozbuffer")

With z-plane:

![Alt text](screenshots/withzbuffer.png?raw=true "withzbuffer")

Without implementing back-face culling:

![Alt text](screenshots/no-culling.png?raw=true "no-culling")

With back-face culling:

![Alt text](screenshots/with-culling.png?raw=true "with-culling")

Comparison between mine and OpenGL's texture mappings:  
(Open = OpenGL, NN = Neirest Neighbour)

![Alt text](screenshots/comparação.png?raw=true "comparação")
