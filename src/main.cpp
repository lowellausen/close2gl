#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "my_obj_loader.cpp"
#include "matrix.cpp"
#include "rastah.cpp"
#include "stb_image.h"
#include "text_maps.cpp"


#include "../imgui/imgui.h"
#include "imgui_impl_glfw_gl3.h"


void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void LoadTextureImage(const char* filename);

float g_ScreenRatio = 1.0f;

float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;
bool g_MiddleMouseButtonPressed = false;


float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance; // Distância da câmera para a origem


GLuint vertex_shader_open_id;
GLuint fragment_shader_open_id;
GLuint vertex_shader_close_id;
GLuint fragment_shader_close_id;
GLuint program_close_id = 0;
GLuint program_open_id = 0;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint model_color_uniform_open;
GLint model_color_uniform_close;
GLint shading_uniform;


GLint frag_type_uniform;
GLuint nonef_index;
GLuint phongf_index;
GLuint textf_index;
GLuint* fragShades[] = {&nonef_index, &phongf_index, &textf_index};

GLint frag_close_uniform;
GLuint notext_index;
GLuint withtext_index;
GLuint* fragCloses[] = {&notext_index, &withtext_index};

GLint vertex_close_uniform;
GLuint noraster_index;
GLuint withraster_index;
GLuint* vertCloses[] = {&noraster_index, &withraster_index};




GLint vertex_type_uniform;
GLuint no_shade_index;
GLuint gourAD_shade_index;
GLuint gourADS_shade_index;
GLuint phong_shade_index;
GLuint* vertShades[] = {&no_shade_index, &gourAD_shade_index, &gourADS_shade_index, &phong_shade_index };




glm::vec3 initial_cam_pos;
glm::vec3 camera_position_c ;
glm::vec3 camera_lookat_l    ;
glm::vec3 camera_view_vector ;
glm::vec3 obj_center;
glm::vec3 camera_up_vector   = glm::vec3(0.0f,1.0f,0.0f);

float* transform_matrix(MyMate* pvm);
float* wdivision(float* transformed);
float* cullit (float* wdivided);
float* good_raster(MyMate* vp, float *vert);
void show_fps(GLFWwindow* window);



MyObj *obj_model = new MyObj();
int goodTris=0;

int close2gl = 1;
int cw = 1;
int lookat = 1;
int poly_mode = 1;
float near_plane = 0.1f;
float far_plane = 3000.0f;
glm::vec3 model_color = glm::vec3(128/256.0, 128/256.0, 128/256.0);
int shading_index = 0;
int frigging_index = 0;
float vfov = 45.0f;
float hfov = 45.0f;
bool cull = true;
bool closeraster = false;
bool apply_text = false;

int filter_mode = 0;

#define NN 0
#define BILI 1
#define MM 2

int screen_width = 800;
int screen_height = 600;

int translate = 1;  //0:rotate 1:translate
int axis = 1; //1:x 2:y 3:z

float delta = 3.141592 / 16;
float inc;

float *depth_buffer;
float *color_buffer;
float *clear_color_buffer;
float *clear_depth_buffer;

GLuint text_id;

glm::mat4 glmModelViewIT;

GLuint sampler_id;

MyMapeur *text_mapper;




int main(int argc, char* argv[])
{
    //carrega o modelo do arquivo de entrada
    obj_model->Load(argv[1]);

    //fprintf(stderr,"argc: %d \t argv2: %s", argc, argv[2]);



    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(screen_width, screen_height, "Assignment3", NULL, NULL);
    //cria janela e todos os seus calllbaks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);


    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600);




    //inicializando os buffer do close 2 raster
    //supomos uma tela absurda pra nunca faltar 2.0fs e pretos
    clear_color_buffer = new float[5000*5000*4];
    clear_depth_buffer = new float[5000*5000];

    //color e depth buffer inicializados com tamanho default da janela!
    color_buffer = new float[800*600*4];
    depth_buffer = new float[800*600];

    for(int i=0;i<=5000*5000;i++){
        clear_depth_buffer[i] = 2000000.0f; //valor maior que qualquer profundidade possível em ndc!!
        clear_color_buffer[i*4+0] = 0.0f;//r  tudo preto
        clear_color_buffer[i*4+1] = 0.0f;//g
        clear_color_buffer[i*4+2] = 0.0f;//b
        clear_color_buffer[i*4+3] = 1.0f;//a
    }

    //carregamos algma textura se precisar!
    if(argc==3)
        if(obj_model->has_text) LoadTextureImage(argv[2]);


    LoadShadersFromFiles();





    /*obj_center = obj_model->bbox_middle;
    glm::vec3 z_dist = obj_model->bbox_max - obj_center;
    g_CameraDistance = z_dist.length()*2.0f;
    initial_cam_pos = glm::vec3(obj_center.x, obj_center.y, g_CameraDistance);*/

    //float z_dist = obj_model->bbox_max - obj_center;

    //posicionamos a camera de forma que o objeto fique centralizado
    obj_center = obj_model->bbox_middle;
    g_CameraDistance = 3*std::max((obj_model->bbox_max.x - obj_model->bbox_min.x)*(1.0f/tan(45.0)),
                          (obj_model->bbox_max.y - obj_model->bbox_min.y)*(1.0f/tan(1.0*45.0)));

   initial_cam_pos = glm::vec3(obj_center.x, obj_center.y, g_CameraDistance);

    camera_position_c = initial_cam_pos;

    camera_lookat_l = MyMate::normalize(obj_center - camera_position_c);

    //incremento para translaçõese rotações
    inc = 0.005f * g_CameraDistance;

   //VAO CLOSEEEE

    GLuint VBO_close;
    glGenBuffers(1, &VBO_close);

    //criamos o vao do close, que trá um vbo de duas coordenadas para os vértices em ndc

    GLuint VAO_close;
    glGenVertexArrays(1, &VAO_close);


    glBindVertexArray(VAO_close);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_close);

    glBufferData(GL_ARRAY_BUFFER, obj_model->NumTris * 12 * sizeof(float), obj_model->Vert, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //criação do vao e vbos do raster close2 gl
    float vertex_data[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f
                           ,1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f};

    float texture_coords[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f
                           ,1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};


    GLuint VBO_raster;
    glGenBuffers(1, &VBO_raster);

     GLuint VBO_rasterT;
    glGenBuffers(1, &VBO_rasterT);

    GLuint VAO_raster;
    glGenVertexArrays(1, &VAO_raster);


    glBindVertexArray(VAO_raster);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_raster);

    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    //coordenadas do quadrado na tela vão no location 1!
    glEnableVertexAttribArray(1);




    glBindBuffer(GL_ARRAY_BUFFER, VBO_rasterT);

    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), texture_coords, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    //coordenadas de mapeamento de textura vão no location 2!
    glEnableVertexAttribArray(2);


    //criamos a tal da textura!
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1,&text_id);
    glBindTexture(GL_TEXTURE_2D, text_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 800, 600);




    //passamos a tal da textura para o fragment shader como Texture!
    glUseProgram(program_close_id);
    glUniform1i(glGetUniformLocation(program_close_id, "Texture"), 1);




    //glBindTexture(GL_TEXTURE_2D, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);



    glUseProgram(program_open_id);
        //criamos o vao do open, que tá um vbo para os vértices e outro para normais



    //VAO OPENNN
     GLuint VBO_open;
    glGenBuffers(1, &VBO_open);

    GLuint VAO_open;
    glGenVertexArrays(1, &VAO_open);


    glBindVertexArray(VAO_open);


    glBindBuffer(GL_ARRAY_BUFFER, VBO_open);

    glBufferData(GL_ARRAY_BUFFER, obj_model->NumTris * 9 * sizeof(float), obj_model->Vert, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);



    GLuint VBO_normal_coefficients_id;

    glGenBuffers(1, &VBO_normal_coefficients_id);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);

    glBufferData(GL_ARRAY_BUFFER, obj_model->NumTris * 9  * sizeof(float), obj_model->Vert_Normal, GL_STATIC_DRAW);

    //glBufferSubData(GL_ARRAY_BUFFER, 0, obj_model->NumTris * 9  * sizeof(float), obj_model->Vert_Normal);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);



    //vbo das coordenadas de texrtura
    GLuint VBO_text_id;

    glGenBuffers(1, &VBO_text_id);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_text_id);

    glBufferData(GL_ARRAY_BUFFER, obj_model->NumTris * 6  * sizeof(float), obj_model->Text_coord, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);

    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);



    glBindVertexArray(0);
    glUseProgram(0);



    //imgui setup
    GLFWwindow* gui_window = glfwCreateWindow(363, 700, "Options", NULL, NULL);
    glfwMakeContextCurrent(gui_window);
    //glfwSwapInterval(1); // Enable vsync

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(gui_window, true);

    // Setup style
    ImGui::StyleColorsDark();


    ImVec4 clear_color = ImVec4(0.6f, 0.3f, 0.6f, 1.00f);


    glfwMakeContextCurrent(window);


    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(gui_window))
    {
        //seta o modo de poligopnos de acordo com a flag
        if(poly_mode==1) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); else if(poly_mode==2)
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE); else glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //limpamos a tela e zbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        //glUseProgram(program_close_id);

        glm::mat4 view;
        MyMate* my_view = new MyMate();

        if(lookat){
            //se o modo ca camera é lookat, calculamos a matriz view correspondente
            //que está sempre olhando para o centro do obj

            view = glm::lookAt(camera_position_c, obj_center, camera_up_vector);
            my_view->view_matrix(camera_position_c, obj_center-camera_position_c, camera_up_vector);

        }else{

            //se o modo da cameraé free, calculamos a view correspondente
            //que está sempre olhando na direção do vetor lookat

            view = glm::lookAt(camera_position_c, camera_position_c + camera_lookat_l, camera_up_vector);
            my_view->view_matrix(camera_position_c, camera_lookat_l, camera_up_vector);
        }



        glm::mat4 projection;
        MyMate* my_projection = new MyMate();

        //calculamos o screen aspect
        g_ScreenRatio = tan(glm::radians(hfov / 2)) / tan(glm::radians(vfov / 2));

        //calculamos as duas projection matrixes
        projection = glm::perspective(glm::radians(vfov), g_ScreenRatio, near_plane, far_plane);
        my_projection->projection_matrix(glm::radians(vfov), g_ScreenRatio, near_plane, far_plane);


        glm::mat4 model = glm::mat4(1.0);


        //transposta da inversa da mv para transformar as normais no rastaah
        glmModelViewIT = glm::transpose(glm::inverse(view*model));
        //glmModelViewIT.transpose();

        //semore indicamos qual é a frontface. necessário em cada laço por problemas com a GUI

        glFrontFace((cw==1)?GL_CW:GL_CCW);

        if(close2gl){
            glDisable(GL_CULL_FACE);
            if(!closeraster){
                //no close fazemos nosso proprio cull

                MyMate* my_model = new MyMate();

                MyMate* pvm = new MyMate();
                //ionstancia uma matriz identidade, e a multiplica po p v m
                pvm->mult(my_projection);
                pvm->mult(my_view);
                pvm->mult(my_model);

                //chama função que multiplica cada vertice pela pvm, e já retira os de w <= 0
                float* transformed = transform_matrix(pvm);
                //usamos o resultado anterior para gerar um novo buffer, com os vértices divididos por w
                float* wdivided = wdivision(transformed);
                free(transformed);
                //com os vértices divididos por w, fazemos o teste de backface culling e removemos os backfaced
                //se cull estpa ativo


                float* culled;
                if(cull)
                    culled = cullit(wdivided);




                glUseProgram(program_close_id);


                            //passamos qual subrotina iremos usar no frag e vertex shader
                glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &noraster_index);
                glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &notext_index);


                //passamos a cor do modelo para os shaders
                glUniform3fv(model_color_uniform_close, 1,glm::value_ptr(model_color));


                glBindVertexArray(VAO_close);

                glBindBuffer(GL_ARRAY_BUFFER, VBO_close);

                //se houve cull passamos os culled para o buffer, se não passamos os divididos por w apenas
                if(cull){
                    glBufferSubData(GL_ARRAY_BUFFER,0, goodTris* 12 * sizeof(float),culled);
                    free(culled);
                }else glBufferSubData(GL_ARRAY_BUFFER,0, goodTris* 12 * sizeof(float),wdivided);

                free(wdivided);

                glDrawArrays(GL_TRIANGLES, 0, goodTris * 3);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);
                glUseProgram(0);
            }
            else{
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            //CLOSETORASTER BEGINS
                //limpa os buffers
                std::memcpy(color_buffer, clear_color_buffer, screen_height*screen_width*4*sizeof(float));
                std::memcpy(depth_buffer, clear_depth_buffer, screen_height*screen_width*sizeof(float));

                //buffah text
              /*for(int j= (screen_width/2)+25; j<=(screen_width/2)+75; j++){
                for(int i=(screen_height/2)-50; i<=(screen_height/2)-25; i++){
                    //printf("\nj:%d\\t\ti:%d", j,i);
              color_buffer[((i * screen_width) +j)  * 4 +0] = 0.0f;
              color_buffer[((i * screen_width) +j)  * 4 +1] = 1.0f;
              color_buffer[((i * screen_width) +j)  * 4 +2] = 0.0f;}}*/




                //cria objeto de raster que fará as funções de raster e guardará os atributos necessários as raster
                //já passando coord de mundo e normais
                MyRastah* rasterizer = new MyRastah(obj_model->NumTris, obj_model->Vert, obj_model->Vert_Normal,
                                                    obj_model->Text_coord);

                //criarei uma matriz mv para salvar coord de camera
                MyMate* mv = new MyMate();
                MyMate* my_model = new MyMate();
                //ionstancia uma matriz identidade, e a multiplica po v m
                //pvm->mult(my_projection);
                mv->mult(my_view);
                mv->mult(my_model);

                //transformamos os vértices pela mv e salvamos os resultados
                rasterizer->get_eye(mv);

                //instanciamos uma viewport
                MyMate* my_vp = new MyMate();
                my_vp->viewport_matrix(0,0,screen_width,screen_height);
                //fazemos a tranformação para o espaço de tela
                //salvando atributos interrantes(talvez) along the way
                rasterizer->project_toscreen(my_projection, my_vp);

                rasterizer->interpolice();
                //printf("\n%d\t%d", goodTris, rasterizer->goodies);

                glUseProgram(program_close_id);

                //passamos qual subrotina iremos usar no frag e vertex shader
                glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &withraster_index);
                glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &withtext_index);

                glBindVertexArray(VAO_raster);
                glActiveTexture(GL_TEXTURE1);
                //passamos o nosso color buffer como uma nova textura!
                glBindTexture(GL_TEXTURE_2D, text_id);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                screen_width, screen_height, GL_RGBA, GL_FLOAT, color_buffer);

                glActiveTexture(GL_TEXTURE0);


                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindVertexArray(0);
                glUseProgram(0);

                //limpamos as estruturas do rasterizer! bad allocs no more!
                rasterizer->destructor();

            }



        }
        else{

            switch(filter_mode){
                case NN:
                glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);break;
                case BILI:
                glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);break;
                case MM:
                glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);break;
            }


            //no open SEMPRE ativamos as flags de cull. por problemas com gui
            if(cull){
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
            else glDisable(GL_CULL_FACE);


            glEnable(GL_DEPTH_TEST);

            glUseProgram(program_open_id);

            //passamos qual subrotina iremos usar no frag e vertex shader
            glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, vertShades[shading_index]);
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, fragShades[frigging_index]);


            //passamos as matrizes pvm para o shader
            glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
            glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            //passamos a cor
            glUniform3fv(model_color_uniform_open, 1,glm::value_ptr(model_color));

             //passamos o tipo de shading
                glUniform1i(shading_uniform, shading_index);


            glBindVertexArray(VAO_open);

            glDrawArrays(GL_TRIANGLES, 0, obj_model->NumTris * 9);

            glBindVertexArray(0);
            glUseProgram(0);

        }




        //calculamos e exibimos fps
        show_fps(window);

        glfwSwapBuffers(window);

        glfwPollEvents();



        //inicio do render da GUI. passamos o contextopara a janela da gui
        glfwMakeContextCurrent(gui_window);
        //omgui test
        ImGui_ImplGlfwGL3_NewFrame();

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {ImGui::Begin("Assignment3");
            //static float f = 0.0f;
            //static int counter = 0;
            //ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f


            //aqui intaciamos sempre os botõoes da GUI. essa é a estratégia usada para testar botões sem
            //nenhum tipo de sinal
            ImGui::Text("Rendering Type:");
            ImGui::RadioButton("Close2GL", &close2gl, 1);
            ImGui::SameLine();
            ImGui::RadioButton("OpenGL", &close2gl, 0);

            ImGui::Checkbox("Close2GL Raster", &closeraster);

            ImGui::Text("\nObject Color");
            ImGui::ColorEdit3("", (float*)&model_color); // Edit 3 floats representing a color

            ImGui::Text("\nCamera Type:");
            ImGui::RadioButton("Look At", &lookat, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Free", &lookat, 0);

            ImGui::Text("\nFront Face:");
            ImGui::RadioButton("CCW", &cw, 0);
            ImGui::SameLine();
            ImGui::RadioButton("CW", &cw, 1);

            ImGui::Checkbox("Back Face Culling", &cull);

            ImGui::PushItemWidth(100);

            ImGui::Text("\nClipping Planes:");
            ImGui::InputFloat("Near", &near_plane, 1.0f, 1.0f);
            ImGui::SameLine();
            ImGui::InputFloat("Far", &far_plane, 1.0f, 1.0f);

            ImGui::Text("\nField of view:");
            ImGui::InputFloat("Vert.", &vfov, 1.0f, 1.0f);
            ImGui::SameLine();
            ImGui::InputFloat("Hor.", &hfov, 1.0f, 1.0f);

            ImGui::PopItemWidth();


            ImGui::Text("\nPolygon Mode:");
            ImGui::RadioButton("Fill", &poly_mode, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Wire", &poly_mode, 2);
            ImGui::SameLine();
            ImGui::RadioButton("Point", &poly_mode, 3);

            ImGui::Text("\nShading Mode:");
            ImGui::RadioButton("None", &shading_index , 0);
            ImGui::SameLine();
            ImGui::RadioButton("GourAD", &shading_index , 1);
            ImGui::SameLine();
            ImGui::RadioButton("GourADS", &shading_index , 2);
            ImGui::SameLine();
            ImGui::RadioButton("Phong", &shading_index , 3);

            ImGui::Text("Texture:");
            ImGui::Checkbox("Apply Texture", &apply_text);

            ImGui::RadioButton("Nearest Neigh.", &filter_mode , NN);
            ImGui::SameLine();
            ImGui::RadioButton("Bilinear", &filter_mode , BILI);
            ImGui::SameLine();
            ImGui::RadioButton("MipMap", &filter_mode , MM);

            //se o shade selecionado não é phong, então o frag shader sempre é none
            frigging_index =  (shading_index==3)? 1:0;

            //testee
            if(apply_text && obj_model->has_text)frigging_index = 2; //teste

            ImGui::Text("\nMovement:");
            ImGui::RadioButton("Translate", &translate , 1);
            ImGui::SameLine();
            ImGui::RadioButton("Rotate", &translate , 0);

            ImGui::RadioButton("X", &axis , 1);
            ImGui::SameLine();
            ImGui::RadioButton("Y", &axis , 2);
            ImGui::SameLine();
            ImGui::RadioButton("Z", &axis , 3);


            //chama o callback do teclado com space
            if (ImGui::Button("Reset"))
                KeyCallback(window, GLFW_KEY_SPACE, 24, GLFW_PRESS, 0);



            //a gui calcula fps sozinha, então uso para comparar com o meu. está ok
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();}


        // Rendering de verdade da gui
        int display_w, display_h;
        glfwGetFramebufferSize(gui_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(gui_window);

        //voltamos o contexto para nossa janela principal
        glfwMakeContextCurrent(window);
    }

    //stbi_image_free(imagedata);

    glfwTerminate();

    return 0;
}


float* transform_matrix(MyMate* pvm){
    //função que transforma os vértices pela mvp e testa w<=0

    float* vert = obj_model->Vert; //vertices do modelo
    int badTris = 0; //numero de triangulos removidos
    int numtris = obj_model->NumTris; //num de triangulos no modelo
    goodTris = numtris; //por enquanto todos tris são bons
    float* transformed = new float[numtris*12]; //cada tri tem 3 vertices em coord homo, numero máximo de resposta

    glm::vec4 v1, v1l, v2, v2l, v3, v3l; //vertices vi do triangulo e elestransformados vil (vi')

    int i =0;
    //i indexa os triangulos que são passados e j itera todos os que são transformados
    //percorremos todos os triangulos
    for(int j=0; j<numtris;j++){
        //lemos cada vértice de um triangulo
        v1 = glm::vec4(vert[9 * j + 0],vert[9 * j + 1],vert[9 * j + 2],1.0f);
        v2 = glm::vec4(vert[9 * j + 3],vert[9 * j + 4],vert[9 * j + 5],1.0f);
        v3 = glm::vec4(vert[9 * j + 6],vert[9 * j + 7],vert[9 * j + 8],1.0f);

        //transformamos cada vertice pela mvp
        v1l = pvm->mult(v1);
        v2l = pvm->mult(v2);
        v3l = pvm->mult(v3);

        //se algum vértice transformado tem w<=0 passamos direto para o próximo triangulo
        if((v1l.w <=0)||(v2l.w <=0)||(v3l.w <=0)
           ||(v1l.z >=far_plane)||(v2l.z >=far_plane)||(v3l.z >=far_plane)
           ||(v1l.z <=near_plane)||(v2l.z <=near_plane)||(v3l.z <=near_plane)
           ||(abs(v1l.x)>=abs(v1l.w))||(abs(v2l.x)>=abs(v2l.w))||(abs(v3l.x)>=abs(v3l.w))
           ){
            badTris++;  //aumenta o  número de de triangulos que serão descartados
            continue;
        }

        /*printf("\nnotraster:\n%f\t%f\t%f", v1l.x,v1l.y,v1l.z);
            printf("\n%f\t%f\t%f", v2l.x,v2l.y,v2l.z);
            printf("\n%f\t%f\t%f\n", v3l.x,v3l.y,v3l.z);*/


        //se o triângulo está ok, passamos ele para a lista de transformados
        transformed[12 * i + 0] = v1l.x;
        transformed[12 * i + 1] = v1l.y;
        transformed[12 * i + 2] = v1l.z; //não precisa de z na real né? too late
        transformed[12 * i + 3] = v1l.w;
        transformed[12 * i + 4] = v2l.x;
        transformed[12 * i + 5] = v2l.y;
        transformed[12 * i + 6] = v2l.z;
        transformed[12 * i + 7] = v2l.w;
        transformed[12 * i + 8] = v3l.x;
        transformed[12 * i + 9] = v3l.y;
        transformed[12 * i + 10] = v3l.z;
        transformed[12 * i + 11] = v3l.w;
        //i é um índice separado para a lista de transformados
        i++;
    }
    goodTris -= badTris;  //menos tris bons
    float* ret = new float [goodTris*12]; //alocamos memórioa no array apenas para os caras bons
    std::memcpy(ret,transformed,goodTris*12*sizeof(float)); //botamos apenas os bons no retorno
    free(transformed);

    return ret;
}

float* wdivision(float* transformed){
    //apenas realiza a divisão por w de todos os vértices

    float* wdivided = new float[goodTris*12];

    for(int i=0; i<goodTris;i++){
        wdivided[12 * i + 0] = transformed[12 * i + 0]/transformed[12 * i + 3];//x //divide x e y dos transformed
        wdivided[12 * i + 1] = transformed[12 * i + 1]/transformed[12 * i + 3];//y // por w dos tranformed
        wdivided[12 * i + 2] = transformed[12 * i + 2]/transformed[12 * i + 3];//z
        wdivided[12 * i + 3] = transformed[12 * i + 3]/transformed[12 * i + 3];//w

        wdivided[12 * i + 4] = transformed[12 * i + 4]/transformed[12 * i + 7];//x
        wdivided[12 * i + 5] = transformed[12 * i + 5]/transformed[12 * i + 7];//y
        wdivided[12 * i + 6] = transformed[12 * i + 7]/transformed[12 * i + 7];//z
        wdivided[12 * i + 7] = transformed[12 * i + 8]/transformed[12 * i + 7];//w

        wdivided[12 * i + 8] = transformed[12 * i + 8]/transformed[12 * i + 11];//x
        wdivided[12 * i + 9] = transformed[12 * i + 9]/transformed[12 * i + 11];//y
        wdivided[12 * i + 10] = transformed[12 * i + 10]/transformed[12 * i + 11];//z
        wdivided[12 * i + 11] = transformed[12 * i + 11]/transformed[12 * i + 11];//w
    }
    return wdivided;
}

float* cullit (float* wdivided){
    //realiza o backface culling de acordo com a pag 92 do red book

    float* culled = new float[goodTris*12]; //tamanho maximo de retorno
    int badTris = 0; //nenhum cara tóximo
    float a=0.0f; // "área" de um, poliogno

    // i indexa os triangulos que serão retornados. j itera todo triangulio que é testado
    //teste aqui fora para não testar sempre após cada área calculada
    if(cw){
        int i =0;
        //para cada triangulo calculamos a área dele
        for(int j=0;j<goodTris;j++){
            a = ((wdivided[12*j+0]*wdivided[12*j+5])-(wdivided[12*j+4]*wdivided[12*j+1]))+
                ((wdivided[12*j+4]*wdivided[12*j+9])-(wdivided[12*j+8]*wdivided[12*j+5]))+
                ((wdivided[12*j+8]*wdivided[12*j+1])-(wdivided[12*j+0]*wdivided[12*j+9]));

            //se elenão estiver frontfacing, vamos ao próximo trig
            if(a>=0){
                badTris++;  //tri backfacing é ruim e descartado
                continue;
            }

            //se estiver ok o passamos aos culled
            culled[12 * i + 0] = wdivided[12*j+0];
            culled[12 * i + 1] = wdivided[12*j+1];
            culled[12 * i + 2] = wdivided[12*j+2];
            culled[12 * i + 3] = wdivided[12*j+3];
            culled[12 * i + 4] = wdivided[12*j+4];
            culled[12 * i + 5] = wdivided[12*j+5];
            culled[12 * i + 6] = wdivided[12*j+6];
            culled[12 * i + 7] = wdivided[12*j+7];
            culled[12 * i + 8] = wdivided[12*j+8];
            culled[12 * i + 9] = wdivided[12*j+9];
            culled[12 * i + 10] = wdivided[12*j+10];
            culled[12 * i + 11] = wdivided[12*j+11];
            i++;
        }
    }else{
        int i=0;
        for(int j=0;j<goodTris;j++){
            a = ((wdivided[12*j+0]*wdivided[12*j+5])-(wdivided[12*j+4]*wdivided[12*j+1]))+
                ((wdivided[12*j+4]*wdivided[12*j+9])-(wdivided[12*j+8]*wdivided[12*j+5]))+
                ((wdivided[12*j+8]*wdivided[12*j+1])-(wdivided[12*j+0]*wdivided[12*j+9]));

            if(a<=0){
                badTris++;  //tri backfacing é ruim e descartado
                continue;
            }
            culled[12 * i + 0] = wdivided[12*j+0];
            culled[12 * i + 1] = wdivided[12*j+1];
            culled[12 * i + 2] = wdivided[12*j+2];
            culled[12 * i + 3] = wdivided[12*j+3];
            culled[12 * i + 4] = wdivided[12*j+4];
            culled[12 * i + 5] = wdivided[12*j+5];
            culled[12 * i + 6] = wdivided[12*j+6];
            culled[12 * i + 7] = wdivided[12*j+7];
            culled[12 * i + 8] = wdivided[12*j+8];
            culled[12 * i + 9] = wdivided[12*j+9];
            culled[12 * i + 10] = wdivided[12*j+10];
            culled[12 * i + 11] = wdivided[12*j+11];
            i++;
        }
    }
    goodTris -= badTris; //diminui os triangulos visiveis
    float* ret = new float[goodTris*12];  //instancia um cara do tamanho certo
    std::memcpy(ret, culled, goodTris*12*sizeof(float));
    free(culled);

    return ret;
}


void LoadShadersFromFiles()
{
    vertex_shader_close_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    vertex_shader_open_id = LoadShader_Vertex("../../src/shader_open.glsl");
    fragment_shader_close_id = LoadShader_Fragment("../../src/shader_fragment.glsl");
    fragment_shader_open_id = LoadShader_Fragment("../../src/shader_fopen.glsl");

    program_close_id = CreateGpuProgram(vertex_shader_close_id, fragment_shader_close_id);
    program_open_id = CreateGpuProgram(vertex_shader_open_id, fragment_shader_open_id);


    model_uniform           = glGetUniformLocation(program_open_id, "model");
    view_uniform            = glGetUniformLocation(program_open_id, "view");
    projection_uniform      = glGetUniformLocation(program_open_id, "projection");
    shading_uniform            = glGetUniformLocation(program_open_id, "shading_index");
    model_color_uniform_open     = glGetUniformLocation(program_open_id, "in_color");
    model_color_uniform_close     = glGetUniformLocation(program_close_id, "in_color");


    vertex_type_uniform = glGetSubroutineUniformLocation(program_open_id, GL_VERTEX_SHADER, "vertex_shading_type");
    no_shade_index = glGetSubroutineIndex(program_open_id, GL_VERTEX_SHADER, "none");
    gourAD_shade_index = glGetSubroutineIndex(program_open_id, GL_VERTEX_SHADER, "gourAD");
    gourADS_shade_index = glGetSubroutineIndex(program_open_id, GL_VERTEX_SHADER, "gourADS");
    phong_shade_index = glGetSubroutineIndex(program_open_id, GL_VERTEX_SHADER, "phong");

    frag_type_uniform = glGetSubroutineUniformLocation(program_open_id, GL_FRAGMENT_SHADER, "frag_shading_type");
    nonef_index = glGetSubroutineIndex(program_open_id, GL_FRAGMENT_SHADER, "nonef");
    phongf_index = glGetSubroutineIndex(program_open_id, GL_FRAGMENT_SHADER, "phongf");
    textf_index = glGetSubroutineIndex(program_open_id, GL_FRAGMENT_SHADER, "textf");

    vertex_close_uniform = glGetSubroutineUniformLocation(program_close_id, GL_VERTEX_SHADER, "vertex_willraster");
    noraster_index = glGetSubroutineIndex(program_close_id, GL_VERTEX_SHADER, "no_raster");
    withraster_index = glGetSubroutineIndex(program_close_id, GL_VERTEX_SHADER, "with_raster");

    frag_close_uniform = glGetSubroutineUniformLocation(program_close_id, GL_FRAGMENT_SHADER, "frag_willtext");
    notext_index = glGetSubroutineIndex(program_close_id, GL_FRAGMENT_SHADER, "no_text");
    withtext_index = glGetSubroutineIndex(program_close_id, GL_FRAGMENT_SHADER, "with_text");


    glUseProgram(program_open_id);

    glUseProgram(0);
}

GLuint LoadShader_Vertex(const char* filename)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);


    LoadShader(filename, vertex_shader_id);

    return vertex_shader_id;
}

GLuint LoadShader_Fragment(const char* filename)
{

    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    LoadShader(filename, fragment_shader_id);

    return fragment_shader_id;
}


void LoadShader(const char* filename, GLuint shader_id)
{
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    glCompileShader(shader_id);

    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    if ( log_length != 0 )
	{
        std::string  output;

		if ( !compiled_ok )
		{
			output += "ERROR: OpenGL compilation of \"";
			output += filename;
			output += "\" failed.\n";
			output += "== Start of compilation log\n";
			output += log;
			output += "== End of compilation log\n";
		}
		else
		{
			output += "WARNING: OpenGL compilation of \"";
			output += filename;
			output += "\".\n";
			output += "== Start of compilation log\n";
			output += log;
			output += "== End of compilation log\n";
		}

        fprintf(stderr, "%s", output.c_str());
	}


    delete [] log;
}

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    GLuint program_id = glCreateProgram();

    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);


    glLinkProgram(program_id);


    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);


        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    screen_height = height;
    screen_width = width;

    //atualiza os close2buffers
    free(color_buffer);
    free(depth_buffer);
    color_buffer = new float[width*height*4];
    depth_buffer = new float[width*height];

    glActiveTexture(GL_TEXTURE1);

    glDeleteTextures(1,&text_id);

    glGenTextures(1,&text_id);
    glBindTexture(GL_TEXTURE_2D, text_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

    glBindTexture(GL_TEXTURE_2D, 0);


    /*glActiveTexture(GL_TEXTURE1);
    glGenTextures(1,&text_id);
    glBindTexture(GL_TEXTURE_2D, text_id);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 800, 600);*/



    g_ScreenRatio = (float)width / height;
}

double g_LastCursorPosX, g_LastCursorPosY;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        g_MiddleMouseButtonPressed = false;
    }
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{

    if (g_LeftMouseButtonPressed)
    {
        float dx = xpos - g_LastCursorPosX; //guardam o movimento vertical e horizontal do cursor
        float dy = ypos - g_LastCursorPosY;

        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;


        /*if (key == GLFW_KEY_X && action == GLFW_PRESS)
        {
            g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
        }

        if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        {
            g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
        }*/

        if (!lookat){
            //realiza as rotações com o mouse
            if(!translate){
                if (axis==3)
                {
                    //rotaciona o uop em torno do view
                    MyMate* rodrigues = new MyMate();
                    //calcula rodrigues para o angulo delta fixo, em torno do eixo dado
                    rodrigues->rotation_matrix(camera_lookat_l, delta*dx*inc*0.005f);

                    //multiplica aquela rodrigues pelo vetor desejado
                    glm::vec4 camera_up_vector4 = rodrigues->mult(glm::vec4(camera_up_vector, 0.0f));
                    //castings vec3 vec4 vec55555
                    camera_up_vector = MyMate::normalize(glm::vec3(camera_up_vector4.x,camera_up_vector4.y,camera_up_vector4.z));
                    //g_AngleZ +=  delta*dx;
                }
                if (axis==2)
                {
                    //rotaciona o view em torno do up
                    MyMate* rodrigues = new MyMate();
                    rodrigues->rotation_matrix(camera_up_vector, delta*dx*inc*0.005f);

                    glm::vec4 camera_lookat_l4 = rodrigues->mult(glm::vec4(camera_lookat_l, 0.0f));

                    camera_lookat_l = MyMate::normalize(glm::vec3(camera_lookat_l4.x,camera_lookat_l4.y,camera_lookat_l4.z));
                    //g_AngleZ +=  delta*dx;
                }
                if (axis==1)
                {
                    //rotaciona o view em torno do side
                    MyMate* rodrigues = new MyMate();
                    rodrigues->rotation_matrix(MyMate::crossproduct(camera_up_vector,camera_lookat_l), delta*dy*inc*0.005f);

                    glm::vec4 camera_lookat_l4 = rodrigues->mult(glm::vec4(camera_lookat_l, 0.0f));

                    camera_lookat_l = MyMate::normalize(glm::vec3(camera_lookat_l4.x,camera_lookat_l4.y,camera_lookat_l4.z));
                    //g_AngleZ +=  delta*dx;
                }
            }
            if(translate){
                //realiza as translações na free. apenas incrementos na pos da camera
                if(axis==1){
                    camera_position_c += inc *dx* glm::cross(camera_lookat_l,camera_up_vector);
                }
                if(axis==2){
                    camera_position_c += inc*dy* camera_up_vector;
                }
                if(axis==3){
                    camera_position_c += inc*dy* camera_lookat_l;
                }

            }
        }
        else{
            if(translate){
                if(axis==1){
                    camera_position_c += inc *dx* MyMate::normalize(glm::cross(camera_lookat_l,camera_up_vector));
                }
                if(axis==2){
                    camera_position_c += inc*dy* camera_up_vector;
                }
                if(axis==3){
                    camera_position_c += inc*dy* camera_lookat_l;
                }
                //translações lookat envolvem incrementos na pos da camera e também recalculo dos vetores da cam
                camera_lookat_l = MyMate::normalize(obj_center - camera_position_c);
                camera_up_vector = -glm::normalize(glm::cross(camera_lookat_l, glm::cross(camera_lookat_l,camera_up_vector)));
                //camera_position_c = g_CameraDistance * MyMate::normalize(camera_position_c);

            }
        }



        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;


        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    float delta = 3.141592 / 16;

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_W)
    {
        float inc = 0.05f;
        camera_position_c += inc* camera_lookat_l;
    }
    if (key == GLFW_KEY_S)
    {
        float inc = 0.05f;
        camera_position_c -= inc* camera_lookat_l;
    }
    if (key == GLFW_KEY_A)
    {
        float inc = 0.05f;
        camera_position_c -= inc* glm::cross(camera_lookat_l,camera_up_vector);
    }
    if (key == GLFW_KEY_D)
    {
        float inc = 0.05f;
        camera_position_c += inc* glm::cross(camera_lookat_l,camera_up_vector);
    }

    if (key == GLFW_KEY_E)
    {
        float inc = 0.05f;
        camera_position_c += inc* camera_up_vector;
    }

    if (key == GLFW_KEY_Q)
    {
        float inc = 0.05f;
        camera_position_c -= inc* camera_up_vector;
    }


    if (key == GLFW_KEY_1 && action == GLFW_PRESS){
        poly_mode = 1;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS){
        poly_mode = 3;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS){
        poly_mode = 2;
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS){
        shading_index= 0;
        frigging_index = 0;
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS){
        shading_index= 1;
        frigging_index = 0;
    }
    if (key == GLFW_KEY_6 && action == GLFW_PRESS){
        shading_index= 2;
        frigging_index = 0;
    }
    if (key == GLFW_KEY_7 && action == GLFW_PRESS){
       shading_index= 3;
       frigging_index = 1;
    }


    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        camera_position_c = initial_cam_pos;
        g_CameraDistance = initial_cam_pos.z;

        camera_up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
        camera_lookat_l = glm::normalize(obj_center - camera_position_c);
    }


    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS){
        int r, g, b;

        std::cout << "\nInsira valor de red:\n";
        std::cin >> r;
        std::cout << "Insira valor de green:\n";
        std::cin >> g;
        std::cout << "Insira valor de blue:\n";
        std::cin >> b;
        std::cout << "Soh valeu!\n";

        model_color = glm::vec3(r/256.0, g/256.0, b/256.0);
    }

     if (key == GLFW_KEY_P && action == GLFW_PRESS){

        std::cout << "\nInsira valor de near plane:\n";
        std::cin >> near_plane;
        std::cout << "Insira valor de far plane:\n";
        std::cin >> far_plane;
        std::cout << "Soh valeu!\n";

    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS){
        if(lookat){
            //camera_position_c = initial_cam_pos;
            //camera_position_c.z /= 2.5f;

                    camera_position_c = initial_cam_pos;
                    g_CameraDistance = initial_cam_pos.z;
            g_AngleX = 0.0f;
            g_AngleY = 0.0f;
            g_AngleZ = 0.0f;
            //g_CameraTheta = 0.0f;
            g_CameraPhi = 0.0f;



            lookat=0;
            //camera_lookat_l    = glm::vec3(0.0f,0.0f,-1.0f);
            camera_lookat_l =  obj_center - camera_position_c;
            camera_lookat_l = glm::normalize(camera_lookat_l);
            glm::vec3 proj = glm::vec3(camera_lookat_l.x,0.0f,camera_lookat_l.z);

            float cosa = dot(glm::vec3(1.0f,0.0f,0.0f), proj);
            g_CameraTheta = (proj.z <=0)? -acos(cosa): acos(cosa);
        }
        else{
            camera_position_c = initial_cam_pos;
            g_CameraDistance = initial_cam_pos.z;
            g_AngleX = 0.0f;
            g_AngleY = 0.0f;
            g_AngleZ = 0.0f;
            g_CameraTheta = 0.0f;
            g_CameraPhi = 0.0f;


            lookat=1;
        }


    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS){
            if(cw){
                cw=0;
                glFrontFace(GL_CCW);
            }else{
                cw=1;
                glFrontFace(GL_CW);
            }
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
            if(close2gl){
                close2gl=0;
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glEnable(GL_DEPTH_TEST);
            }else{
                close2gl=1;
                glDisable(GL_CULL_FACE);
            }
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    g_CameraDistance -= 0.9f*yoffset;

    if (g_CameraDistance < 0.0f)
        g_CameraDistance = 0.0f;
}

void show_fps(GLFWwindow* window){

    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20,(close2gl)? "Close2GL: %.2f fps":"OpenGL: %.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }
    glfwSetWindowTitle(window, buffer);
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char * imagedata;
    imagedata = stbi_load(filename, &width, &height, &channels, 3);


    if ( imagedata == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    if(obj_model->has_text){
        text_mapper = new MyMapeur(imagedata, width, height);

        text_mapper->init_mipsmaps();
    }

    //text_mapper->setall(text_mapper->mipsmaps[5].imagedata,text_mapper->mipsmaps[5].width,text_mapper->mipsmaps[5].height);
    //text_mapper->init_mipsmaps();

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slide 160 do documento "Aula_20_e_21_Mapeamento_de_Texturas.pdf"
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Parâmetros de amostragem da textura. Falaremos sobre eles em uma próxima aula.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, text_mapper->width, text_mapper->height, 0, GL_RGB, GL_UNSIGNED_BYTE, text_mapper->imagedata);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(0, sampler_id);

    glUseProgram(program_open_id);
    glUniform1i(glGetUniformLocation(program_open_id, "Texture"), 0);


    //stbi_image_free(data);

}
