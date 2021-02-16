#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <time.h>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

#include "matrix.cpp"
#include "text_maps.cpp"

#define N_INTERPIE 16

#define XCREEN 0
#define YCREEN 1
#define WCREEN 2
#define XNORMAL 3
#define YNORMAL 4
#define ZNORMAL 5
#define XEYE 6
#define YEYE 7
#define ZEYE 8
#define ZNDC 9
#define RCOL 10
#define GCOL 11
#define BCOL 12
#define STEXT 13
#define TTEXT 14
#define ONE 15

//prova suprena de poor programming!
//o raster utiliza várias variáveis de controle da main
//eis elas como externs!
extern int shading_index;
extern glm::vec3 model_color;
extern float* color_buffer;
extern float* depth_buffer;
extern int screen_width;
extern int screen_height;
extern int poly_mode;
extern int cw;
//transposta da invera da modelview para transformar normais!
extern glm::mat4 glmModelViewIT;
extern float near_plane;
extern float far_plane;
extern int filter_mode;
extern bool apply_text;

#define NN 0
#define BILI 1
#define MM 2

extern MyMapeur *text_mapper;

class MyRastah
{
public:
    float* world; //original vertices
    float* eye; //eye coords
    float* screen; //device coords
    float* ndc; //normalized device coordinates
    float* normals; //normals
    float* colors; //colors
    float* text;

    //triangulos visíveis
    int goodies;

    //controle do wireframe
    bool drawfull = false;

    float deltas; //delta s da texturapara mipmap
    float deltat; //delta t da texturapara mipamap

    MyRastah(int goodies, float *vert, float* normies, float* texts){
        this->goodies = goodies;

                        //salva coordenadas de mundo e normais no rasterizer
        this->normals = new float[goodies*12];
        this->world = new float[goodies*12];
        this->text = new float[goodies*6];
        std::memcpy(this->text, texts, goodies*6*sizeof(float));

        for(int i=0; i<this->goodies;i++){
            this->world[12 * i + 0] = vert[9*i+0];
            this->world[12 * i + 1] = vert[9*i+1];
            this->world[12 * i + 2] = vert[9*i+2];
            this->world[12 * i + 3] = 1.0f;
            this->world[12 * i + 4] = vert[9*i+3];
            this->world[12 * i + 5] = vert[9*i+4];
            this->world[12 * i + 6] = vert[9*i+5];
            this->world[12 * i + 7] = 1.0f;
            this->world[12 * i + 8] = vert[9*i+6];
            this->world[12 * i + 9] = vert[9*i+7];
            this->world[12 * i + 10] = vert[9*i+8];
            this->world[12 * i + 11] = 1.0f;

            this->normals[12 * i + 0] = normies[9*i+0];
            this->normals[12 * i + 1] = normies[9*i+1];
            this->normals[12 * i + 2] = normies[9*i+2];
            this->normals[12 * i + 3] = 0.0f;
            this->normals[12 * i + 4] = normies[9*i+3];
            this->normals[12 * i + 5] = normies[9*i+4];
            this->normals[12 * i + 6] = normies[9*i+5];
            this->normals[12 * i + 7] = 0.0f;
            this->normals[12 * i + 8] = normies[9*i+6];
            this->normals[12 * i + 9] = normies[9*i+7];
            this->normals[12 * i + 10] = normies[9*i+8];
            this->normals[12 * i + 11] = 0.0f;

        }
    }

    void destructor(){
        free(world);
        free(eye);
        free(screen);
        free(ndc);
        free(normals);
        free(text);
        //free(colors);
    }

    void project_toscreen(MyMate* proj, MyMate* vp){
    //pega vertices em coord de camera e os transorma em coordenadas de tela com a viewport

    //Primeiro transofrma apenas pela projecton, para realizar operação de clipping
    //Ddepois pela viewport para pbter oord de tela

        float* normal_remains = new float[this->goodies*12]; //cada tri tem 3 vertices em coord homo, numero máximo de resposta
        float* world_remains = new float[this->goodies*12];
        float* eye_remains = new float[this->goodies*12];
        float* text_remains = new float[this->goodies*6];
        ndc = new float[this->goodies*3]; //apenas o z??
        screen = new float[this->goodies*12];

        int badTris =0;

        float a=0.0f;


        glm::vec4 v1, v1l, v2, v2l, v3, v3l, v1s, v2s, v3s,n1,n2,n3; //vertices vi do triangulo e elestransformados vil (vi')

        int j=0; //index dos caras que são mantidos firme e forte
        for(int i=0; i<this->goodies;i++){
            //lemos os triangulos em eye coordinates
            v1 = glm::vec4(this->eye[12 * i + 0],this->eye[12 * i + 1],this->eye[12 * i + 2],this->eye[12 * i + 3]);
            v2 = glm::vec4(this->eye[12 * i + 4],this->eye[12 * i + 5],this->eye[12 * i + 6],this->eye[12 * i + 7]);
            v3 = glm::vec4(this->eye[12 * i + 8],this->eye[12 * i + 9],this->eye[12 * i + 10],this->eye[12 * i + 11]);

            //transformamos cada vertice pela projection
            v1l = proj->mult(v1);
            v2l = proj->mult(v2);
            v3l = proj->mult(v3);


            //faz clipping sem todas as dimensões possíveis e imaginaveis
            if((v1l.w <=0)||(v2l.w <=0)||(v3l.w <=0)
           ||(v1l.z >=far_plane)||(v2l.z >=far_plane)||(v3l.z >=far_plane)
           ||(v1l.z <=near_plane)||(v2l.z <=near_plane)||(v3l.z <=near_plane)
           ||(abs(v1l.x)>=abs(v1l.w))||(abs(v2l.x)>=abs(v2l.w))||(abs(v3l.x)>=abs(v3l.w))
           ){
            badTris++;  //aumenta o  número de de triangulos que serão descartados
            continue;
            }

            a = ((v1l.x/v1l.w*v2l.y/v2l.w-v2l.x/v2l.w*v1l.y/v1l.w)
                 +(v2l.x/v2l.w*v3l.y/v3l.w-v3l.x/v3l.w*v2l.y/v2l.w)
                 +(v3l.x/v3l.w*v1l.y/v1l.w-v1l.x/v1l.w*v3l.y/v3l.w));
            if(((cw==1) && a>=0.0f)||((cw==0) && a<=0.0f)){
                badTris++;  //aumenta o  número de de triangulos que serão descartados
                continue;
            }
            //printf("\n%f\t%f\t%f\n", v1l.z/v1l.w, v2l.z/v2l.w, v3l.z/v3l.w);

            /*printf("\nraster:\n%f\t%f\t%f", v1l.x,v1l.y,v1l.z);
            printf("\n%f\t%f\t%f", v2l.x,v2l.y,v2l.z);
            printf("\n%f\t%f\t%f\n", v3l.x,v3l.y,v3l.z);*/

            //cria vec4s das normais para transformalas pela inversa da modelview
            //obtendo normais em coords de camera para calculo de iluilu
            n1 = glmModelViewIT*glm::vec4(this->normals[12 * i + 0],this->normals[12 * i + 1],this->normals[12 * i + 2],this->normals[12 * i + 3]);
            n2 = glmModelViewIT*glm::vec4(this->normals[12 * i + 4],this->normals[12 * i + 5],this->normals[12 * i + 6],this->normals[12 * i + 7]);
            n3 = glmModelViewIT*glm::vec4(this->normals[12 * i + 8],this->normals[12 * i + 9],this->normals[12 * i + 10],this->normals[12 * i + 11]);


            //guarda a normal do cara que passou
            normal_remains[12 * j + 0] = n1.x;
            normal_remains[12 * j + 1] = n1.y;
            normal_remains[12 * j + 2] = n1.z;
            normal_remains[12 * j + 3] = n1.w;
            normal_remains[12 * j + 4] = n2.x;
            normal_remains[12 * j + 5] = n2.y;
            normal_remains[12 * j + 6] = n2.z;
            normal_remains[12 * j + 7] = n2.w;
            normal_remains[12 * j + 8] = n3.x;
            normal_remains[12 * j + 9] = n3.y;
            normal_remains[12 * j + 10] = n3.z;
            normal_remains[12 * j + 11] = n3.w;
            //guardamos as coord de mundo dos triangulos sobreviventes
            world_remains[12 * j + 0] = this->world[12*i+0];
            world_remains[12 * j + 1] = this->world[12*i+1];
            world_remains[12 * j + 2] = this->world[12*i+2];
            world_remains[12 * j + 3] = this->world[12*i+3];
            world_remains[12 * j + 4] = this->world[12*i+4];
            world_remains[12 * j + 5] = this->world[12*i+5];
            world_remains[12 * j + 6] = this->world[12*i+6];
            world_remains[12 * j + 7] = this->world[12*i+7];
            world_remains[12 * j + 8] = this->world[12*i+8];
            world_remains[12 * j + 9] = this->world[12*i+9];
            world_remains[12 * j + 10] = this->world[12*i+10];
            world_remains[12 * j + 11] = this->world[12*i+11];
            //guardamos os eye coord sobreviventes tbm
            eye_remains[12 * j + 0] = this->eye[12*i+0];
            eye_remains[12 * j + 1] = this->eye[12*i+1];
            eye_remains[12 * j + 2] = this->eye[12*i+2];
            eye_remains[12 * j + 3] = this->eye[12*i+3];
            eye_remains[12 * j + 4] = this->eye[12*i+4];
            eye_remains[12 * j + 5] = this->eye[12*i+5];
            eye_remains[12 * j + 6] = this->eye[12*i+6];
            eye_remains[12 * j + 7] = this->eye[12*i+7];
            eye_remains[12 * j + 8] = this->eye[12*i+8];
            eye_remains[12 * j + 9] = this->eye[12*i+9];
            eye_remains[12 * j + 10] = this->eye[12*i+10];
            eye_remains[12 * j + 11] = this->eye[12*i+11];

            text_remains[6 * j + 0] = this->text[6 * i + 0];
            text_remains[6 * j + 1] = this->text[6 * i + 1];
            text_remains[6 * j + 2] = this->text[6 * i + 2];
            text_remains[6 * j + 3] = this->text[6 * i + 3];
            text_remains[6 * j + 4] = this->text[6 * i + 4];
            text_remains[6 * j + 5] = this->text[6 * i + 5];

            //salvamos o z em NDC para futura interpolação
            ndc[3 * j + 0] = v1l.z/v1l.w;
            ndc[3 * j + 1] = v2l.z/v2l.w;
            ndc[3 * j + 2] = v3l.z/v3l.w;

            //multiplicamos os tris pós projection pela viewport, para obter coords de tela
            v1s = vp->mult(v1l);
            v2s = vp->mult(v2l);
            v3s = vp->mult(v3l);

            this->screen[12 * j + 0] = v1s.x/v1s.w; //divide x y e z por w e mantem o w ´paara uso na interpolação
            this->screen[12 * j + 1] = v1s.y/v1s.w; //divisão perspectiva
            this->screen[12 * j + 2] = v1s.z/v1s.w;
            this->screen[12 * j + 3] = v1s.w;
            this->screen[12 * j + 4] = v2s.x/v2s.w;
            this->screen[12 * j + 5] = v2s.y/v2s.w;
            this->screen[12 * j + 6] = v2s.z/v2s.w;
            this->screen[12 * j + 7] = v2s.w;
            this->screen[12 * j + 8] = v3s.x/v3s.w;
            this->screen[12 * j + 9] = v3s.y/v3s.w;
            this->screen[12 * j + 10] = v3s.z/v3s.w;
            this->screen[12 * j + 11] = v3s.w;

            //printf("\n%f\t%f", v1l.w, v1s.w);
            /*printf("\n%f\t%f\t%f", v3l.x/v3l.w,v3s.x/v3s.w,400+(v3l.x/v3l.w*400));
               printf("\n%f\t%f\t%f\n", v3l.y/v3l.w,v3s.y/v3s.w,300+(-v3l.y/v3l.w*300));*/

            j++;
        }

        this->goodies -= badTris;


        //liberamos os atributos atuais para salvar apenas os visíveis
        free(this->normals);
        free(this->world);
        free(this->eye);
        free(this->text);
        //alocamos memória e salvamos os visíveis no lugar
        this->normals = new float[this->goodies*12];
        this->eye =new float[this->goodies*12];
        this->world =new float[this->goodies*12];
        this->text = new float[this->goodies*6];
        std::memcpy(this->normals, normal_remains, this->goodies*12*sizeof(float));
        std::memcpy(this->eye, eye_remains, this->goodies*12*sizeof(float));
        std::memcpy(this->world, world_remains, this->goodies*12*sizeof(float));
        std::memcpy(this->text, text_remains, this->goodies*6*sizeof(float));
        //liberamos os intermediários
        free(world_remains);
        free(eye_remains);
        free(normal_remains);
        free(text_remains);


        return;
    }


   void get_eye(MyMate* mv){
        this->eye = new float[this->goodies*12];

        glm::vec4 v1, v1l, v2, v2l, v3, v3l;

        for(int i=0; i<this->goodies;i++){
            //lemos cada vértice de um triangulo
            v1 = glm::vec4(this->world[12 * i + 0],this->world[12 * i + 1],this->world[12 * i + 2],this->world[12 * i + 3]);
            v2 = glm::vec4(this->world[12 * i + 4],this->world[12 * i + 5],this->world[12 * i + 6],this->world[12 * i + 7]);
            v3 = glm::vec4(this->world[12 * i + 8],this->world[12 * i + 9],this->world[12 * i + 10],this->world[12 * i + 11]);

            //transformamos cada vertice pela mvp
            v1l = mv->mult(v1);
            v2l = mv->mult(v2);
            v3l = mv->mult(v3);

            this->eye[12 * i + 0] = v1l.x;
            this->eye[12 * i + 1] = v1l.y;
            this->eye[12 * i + 2] = v1l.z;
            this->eye[12 * i + 3] = v1l.w;
            this->eye[12 * i + 4] = v2l.x;
            this->eye[12 * i + 5] = v2l.y;
            this->eye[12 * i + 6] = v2l.z;
            this->eye[12 * i + 7] = v2l.w;
            this->eye[12 * i + 8] = v3l.x;
            this->eye[12 * i + 9] = v3l.y;
            this->eye[12 * i + 10] = v3l.z;
            this->eye[12 * i + 11] = v3l.w;
        }

    }


    void interpolice(){


        for(int i =0; i<this->goodies; i++){
            //arrays com os valores a serem interpolados para cada vértice
            float* interpies1 = get_interpie(i, 0);
            float* interpies2 = get_interpie(i, 1);
            float* interpies3 = get_interpie(i, 2);

            wcreamdivide(interpies1);
            wcreamdivide(interpies2);
            wcreamdivide(interpies3);



            //printf("\n%f", interpies1[YCREEN]);
            //float aa = interpies1[YCREEN];
            //sorty(interpies1, interpies2, interpies3);
            //printf("\n%f\n", interpies1[YCREEN]);

            //faz o ordenamento dos vértices em Y
            if (interpies1[YCREEN] > interpies2[YCREEN]){
                if (interpies2[YCREEN] > interpies3[YCREEN]){
                    std::swap(interpies1, interpies3);
                }else{
                    std::swap(interpies1, interpies2);
                    if (interpies2[YCREEN] > interpies3[YCREEN]){
                        std::swap(interpies2, interpies3);
                    }
                }
            }else if (interpies2[YCREEN] > interpies3[YCREEN]){
                std::swap(interpies2, interpies3);
                if (interpies1[YCREEN] > interpies2[YCREEN]){
                    std::swap(interpies1, interpies2);
                }
            }
            //fim do ordenamento


            drawtriangle(interpies1, interpies2, interpies3);



            free(interpies1);
            free(interpies2);
            free(interpies3);
        }



    }

    void wcreamdivide(float* inter){
        //faz a primeira divisão por w
        //divide todos os caras que devem ser interpolados hiperbolicamente
        inter[XNORMAL] /= inter[WCREEN];
        inter[YNORMAL] /= inter[WCREEN];
        inter[ZNORMAL] /= inter[WCREEN];

        inter[XEYE] /= inter[WCREEN];
        inter[YEYE] /= inter[WCREEN];
        inter[ZEYE] /= inter[WCREEN];

        inter[RCOL] /= inter[WCREEN];
        inter[GCOL] /= inter[WCREEN];
        inter[BCOL] /= inter[WCREEN];

        inter[ZNDC] /= inter[WCREEN];

        inter[STEXT] /= inter[WCREEN];
        inter[TTEXT] /= inter[WCREEN];

        inter[ONE] /= inter[WCREEN];
    }

    void wtildivide(float* inter){
        //faz a divisão divisão por w (1/w)
        //divide todos os caras que devem ser interpolados hiperbolicamente
        inter[XNORMAL] /= inter[ONE];
        inter[YNORMAL] /= inter[ONE];
        inter[ZNORMAL] /= inter[ONE];

        inter[XEYE] /= inter[ONE];
        inter[YEYE] /= inter[ONE];
        inter[ZEYE] /= inter[ONE];

        inter[RCOL] /= inter[ONE];
        inter[GCOL] /= inter[ONE];
        inter[BCOL] /= inter[ONE];

        inter[ZNDC] /= inter[ONE];

        inter[STEXT] /= inter[ONE];
        inter[TTEXT] /= inter[ONE];

        inter[ONE] /= inter[ONE];
    }


    //monsta o array com valores a serem interpolados!
    //vértice v do triângulo t
    float* get_interpie(int t, int v){
        float* ret = new float[N_INTERPIE];

        ret[XCREEN] = this->screen[12*t+(4*v)];
        ret[YCREEN] = this->screen[12*t+(4*v)+1];
        ret[WCREEN] = this->screen[12*t+(4*v)+3];

        ret[XNORMAL] = this->normals[12*t+(4*v)];
        ret[YNORMAL] = this->normals[12*t+(4*v)+1];
        ret[ZNORMAL] = this->normals[12*t+(4*v)+2];

        ret[XEYE] = this->eye[12*t+(4*v)];
        ret[YEYE] = this->eye[12*t+(4*v)+1];
        ret[ZEYE] = this->eye[12*t+(4*v)+2];

        ret[STEXT] = this->text[6*t+(2*v)+0];
        ret[TTEXT] = this->text[6*t+(2*v)+1];



        ret[ZNDC] = this->ndc[3*t+v];

        if(shading_index==2 || shading_index==1){//gouraud case
            glm::vec4 eyeG = glm::vec4(ret[XEYE], ret[YEYE], ret[ZEYE], 1.0f);
            glm::vec4 normaG = glm::vec4(ret[XNORMAL], ret[YNORMAL], ret[ZNORMAL], 0.0f);
            //printf("\nNormal Tri %d Vert %d: %f\t%f\t%f",t,v, normaG.x, normaG.y, normaG.z);
            glm::vec3 color = calc_color(eyeG, normaG);
            ret[RCOL] = color.r;
            ret[GCOL] = color.g;
            ret[BCOL] = color.b;
            /*normaG = glm::normalize(normaG);
            ret[RCOL] = normaG.x/2 +0.5f;
            ret[GCOL] = normaG.y/2 +0.5f;
            ret[BCOL] = normaG.z/2 +0.5f;*/
        }else{//caso sem shading ooou phong
            ret[RCOL] = model_color.r;
            ret[GCOL] = model_color.g;
            ret[BCOL] = model_color.b;
        }


        /*//super teste uma cor pra cada vértice trop bonito
        if(v==0){
            ret[RCOL] = 1.0f;
            ret[GCOL] = 0.0f;
            ret[BCOL] = 0.0f;
        }
        if(v==1){
            ret[RCOL] = 0.0f;
            ret[GCOL] = 1.0f;
            ret[BCOL] = 0.0f;
        }
        if(v==2){
            ret[RCOL] = 0.0f;
            ret[GCOL] = 0.0f;
            ret[BCOL] = 1.0f;
        }*/

        ret[ONE] = 1.0f;

        return ret;
    }

    glm::vec3 calc_color(glm::vec4 position_eye, glm::vec4 normal){
        //somente chama o calculo de cor de acordo com o gouraud desejado
        //grande gama de gourauds atualmente em uma quantidade de 2 _dois_
        //gouraud ad e gouraux ads goureux
        if(shading_index==1){
            return gouraud_ad(position_eye, normal);
        }else{
            return phongouraud(position_eye, normal);
        }
    }

    glm::vec3 phongouraud(glm::vec4 position_eye, glm::vec4 normal){
        //calculo de ilu que serve tanto para gouraud ads quanto para phong
        //para gouraud é chamada uma vez para cada vértice
        //em phong vai ser chamado uma vez em cada fragmento

        //todos os coeficientes são a cord do modelo
        glm::vec3 Kd = model_color;
        glm::vec3 Ks = model_color;
        glm::vec3 Ka = model_color;
        //coef de especualirade phongs fixo em 5
        float q = 5.0f;

        //normalizamos a normal recebida
        //pq ela veio de uma multiplicação pela trans da mv então pode não estar normalizada
        //pode ter vindo de uma interpolação tbm pior ainda
        normal = glm::normalize(normal);

        //fonte de luz pontual full brightness
        glm::vec3 light_spectrum = glm::vec3(1.0,1.0,1.0);
        //ambient light fraquita
        glm::vec3 ambient_light_spectrum = glm::vec3(0.2,0.2,0.2);

        //camera é a origem já que estamos em ccs né
        glm::vec4 camera_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        //ponto p onde a luz será calculada é o position eye recebido
        //eye pq ccs não pq essa é a posição do observador
        //pq n é
        glm::vec4 p = position_eye;

        //posição da fonte de luz pontual é 2 2 2 acima e a frente e à direita da camera
        glm::vec4 light_src_position = glm::vec4(2.0f,2.0f,2.0f,1.0f);

        //começando o c[alculo de verdade!!
        //vetor ponto->luz
        glm::vec4 l = normalize(light_src_position - p);
        //vetor ponto->obervador
        glm::vec4 v = glm::normalize(camera_position - p);
        //vetor de reflexão
        glm::vec4 r = -l + 2.0f * normal * glm::dot(normal, l);
        float diffuse = std::max( 0.0f , glm::dot(normal, l));//calculo difuso
        float specular  = pow( std::max( 0.0f, glm::dot(r, v)), q);//calculo especular
        //soma ponderada resultando na cor final do ponto
        return(Ka*ambient_light_spectrum+Kd*light_spectrum*diffuse+Ks*light_spectrum*specular);
    }

    glm::vec3 gouraud_ad(glm::vec4 position_eye, glm::vec4 normal){
        glm::vec3 Kd = model_color;
        glm::vec3 Ks = model_color;
        glm::vec3 Ka = model_color;

        normal = glm::normalize(normal);

        glm::vec3 light_spectrum = glm::vec3(1.0,1.0,1.0);
        glm::vec3 ambient_light_spectrum = glm::vec3(0.2,0.2,0.2);

        glm::vec4 camera_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 p = position_eye;

        glm::vec4 light_src_position = glm::vec4(2.0f,2.0f,2.0f,1.0f);
        glm::vec4 l = normalize(light_src_position - p);

        float diffuse = std::max( 0.0f , glm::dot(normal, l));

        return ( Ka * ambient_light_spectrum + Kd * light_spectrum * diffuse);
    }


    void drawtriangle(float* v1, float* v2, float* v3){
        //trata caso fora do buffer
        v1[YCREEN] = floor(v1[YCREEN]);v2[YCREEN] = floor(v2[YCREEN]);v3[YCREEN] = floor(v3[YCREEN]);
        if(v1[YCREEN]>=screen_height)v1[YCREEN]-=1;
        if(v2[YCREEN]>=screen_height)v2[YCREEN]-=1;
        if(v3[YCREEN]>=screen_height)v3[YCREEN]-=1;

        v1[XCREEN] = floor(v1[XCREEN]);v2[XCREEN] = floor(v2[XCREEN]);v3[XCREEN] = floor(v3[XCREEN]);
        if(v1[XCREEN]>=screen_width)v1[XCREEN]-=1;
        if(v2[XCREEN]>=screen_width)v2[XCREEN]-=1;
        if(v3[XCREEN]>=screen_width)v3[XCREEN]-=1;


        //caso modo point apenas desenhamos os tre vértices
        if(poly_mode==3){
            drawPoint(v1);drawPoint(v2);drawPoint(v3);return;
        }
        //caso o triangulo seja horizontal embaixo:
       if(v2[YCREEN]==v3[YCREEN]){
            if(poly_mode==2)this->drawfull=true; //se é omodo wireframe vamos desenhar toda a aresta horizontal
            if(v2[XCREEN]<v3[XCREEN]) fillBottomFlatTriangle(v1, v2, v3);
            else                      fillBottomFlatTriangle(v1, v3, v2);
        }
        else if(v1[YCREEN]==v2[YCREEN]){ //triangulo reto em cima
            if(poly_mode==2)this->drawfull=true; //se é omodo wireframe vamos desenhar toda a aresta horizontal
            if(v1[XCREEN]<v2[XCREEN]) fillTopFlatTriangle(v1, v2, v3);
            else                      fillTopFlatTriangle(v2, v1, v3);
        }else{//caso genérico, dividimos em dois triangulos retinhos

            float* v4 = midVert(v1,v3, v2[YCREEN]-v1[YCREEN]); //cria vertice intermediário na altura de v2
            if(poly_mode==2)this->drawfull=false; //não precisamos desenhar todas as arestas, apenas o primeiro ponto
            if(v2[XCREEN]<v4[XCREEN]){
                fillBottomFlatTriangle(v1, v2, v4);
                fillTopFlatTriangle(v2, v4, v3);
            }else{
                fillBottomFlatTriangle(v1, v4, v2);
                fillTopFlatTriangle(v4, v2, v3);
            }
            free(v4);
        }
        /*v1[YCREEN] = 500.0f;
        v1[XCREEN] = 50.0f;

        v2[YCREEN] = 100.0f;
        v2[XCREEN] = 100.0f;

        v3[YCREEN] = 500.0f;
        v3[XCREEN] = 150.0f;
        fillBottomFlatTriangle(v1, v2, v3);
        return;*/
    }

    void fillBottomFlatTriangle(float* v1, float* v2, float* v3){
        //vertices intermediários das interpos LIneares
        float* live1;//da esquerda
        float* live2;//da direita     ordem garantida pela funcção que chama


        int firsty = (int)v1[YCREEN]; //l'y le plus haut
        int lasty = (int)v3[YCREEN]; //l'y le moins haut


        for(int scanline = lasty ; scanline >= firsty; scanline--){
            int dy = scanline - firsty;

            //printcolor(v1, "Cor do v1:");
            //printcolor(v2, "Cor do v2:");

            live1 = midVert(v1, v2, dy); //interpola as duas arestas
            live2 = midVert(v1, v3, dy);

            //printcolor(live1, "Cor da inter v1->v2:");


            drawline(live1, live2);
            this->drawfull = false;

            free(live1);
            free(live2);
        }

    }

    void fillTopFlatTriangle(float* v1, float* v2, float* v3){
        //vertices intermediários das interpos LIneares
        float* live1;//da esquerda
        float* live2;//da direita     ordem garantida pela funcção que chama

        int firsty = (int)v1[YCREEN]; //l'y le plus haut
        int lasty = (int)v3[YCREEN]; //l'y le moins haut

        for(int scanline = firsty ; scanline <= lasty; scanline++){
            int dy = scanline - firsty;

            live1 = midVert(v1, v3, dy); //interpola as duas arestas
            live2 = midVert(v2, v3, dy);


            drawline(live1, live2);
            this->drawfull = false;

            free(live1);
            free(live2);
        }
    }

    void drawline(float* v1, float* v2){
        //desenha uma scanline fazendo interpolações lineares de dois vértices já interpolados
        //vai de v1 para v2 ----->>>>>
    	int premierx = (int)v1[XCREEN];
        int dernierx = (int)v2[XCREEN];

        float* bive; //vértice da interpolação BIlinear

        deltas = 0.0f;
        deltat = 0.0f;

        //wrap around or something
        bive = midPoint(v1, v2, -1);

        float olds = bive[STEXT];
        float oldt = bive[TTEXT];


        if(((poly_mode==2) && drawfull)||(poly_mode==1)){
        for(int curx = premierx; curx<=dernierx; curx++){
            bive = midPoint(v1, v2, curx-premierx); //interpolamos linearmente

            //foi para dentro do drawpoint for reasons!
            //wtildivide(bive); //realiza a segunda divisão por w (1/w) para interpolação corretx

            //calcula os deltas de s e t da textura após a interpolação
            this->deltas = fabs(bive[STEXT] - olds);
            this->deltat = fabs(bive[TTEXT] - oldt);
            olds=bive[STEXT];
            oldt=bive[TTEXT];

            drawPoint(bive);

            free(bive);
        }}
        else{
            bive = midPoint(v1, v2,0); //interpolamos apenas o primeiro e ultimo ponto
            //foi para dentro do drawpoint for reasons!
            //wtildivide(bive); //realiza a segunda divisão por w (1/w) para interpolação corretx
            drawPoint(bive);
            free(bive);

            bive = midPoint(v1, v2, dernierx-premierx); //interpolamos apenas o primeiro e ultimo ponto
            //foi para dentro do drawpoint for reasons!
            //wtildivide(bive); //realiza a segunda divisão por w (1/w) para interpolação corretx
            drawPoint(bive);
            free(bive);
        }
    }

    float* midVert(float* v1, float* v2, int dy){
        //retorna um vértice interpolado entre dois vértices, em uma altura y
        //usado para gerar as interpo lineares que serão usadas nas bilineares
        float alpha = v2[YCREEN] - v1[YCREEN];
        if(alpha!=0.0f)alpha = dy/alpha;  //calculo de alpha da interpolação

        float beta = (1 - alpha);  // complemento do alpha

        float* ret =  new float[N_INTERPIE];
        ret[YCREEN] = alpha*v2[YCREEN] + beta*v1[YCREEN];
        ret[XCREEN] = alpha*v2[XCREEN] + beta*v1[XCREEN];//atributos interpoladoes entre vértices v1 e v2
        ret[WCREEN] = alpha*v2[WCREEN] + beta*v1[WCREEN];
        ret[ZNDC] = alpha*v2[ZNDC] + beta*v1[ZNDC];


        ret[ONE] = alpha*v2[ONE] + beta*v1[ONE];

        ret[RCOL] = alpha*v2[RCOL] + beta*v1[RCOL];
        ret[GCOL] = alpha*v2[GCOL] + beta*v1[GCOL];
        ret[BCOL] = alpha*v2[BCOL] + beta*v1[BCOL];

        ret[XNORMAL] = alpha*v2[XNORMAL] + beta*v1[XNORMAL];
        ret[YNORMAL] = alpha*v2[YNORMAL] + beta*v1[YNORMAL];
        ret[ZNORMAL] = alpha*v2[ZNORMAL] + beta*v1[ZNORMAL];

        ret[XEYE] = alpha*v2[XEYE] + beta*v1[XEYE];
        ret[YEYE] = alpha*v2[YEYE] + beta*v1[YEYE];
        ret[ZEYE] = alpha*v2[ZEYE] + beta*v1[ZEYE];

        ret[STEXT] = alpha*v2[STEXT] + beta*v1[STEXT];
        ret[TTEXT] = alpha*v2[TTEXT] + beta*v1[TTEXT];

        return ret;
    }

    float* midPoint(float* v1, float* v2, int dx){
        //retorna um vértice interpolado entre dois vértices, em uma largura x
        //usado para gerar as interpo bilineares que serão usadas nas scanlines
        float alpha = v2[XCREEN] - v1[XCREEN];
        if(alpha!=0.0f)alpha = dx/alpha;  //calcula de alpha da interpolação

        float beta = (1 - alpha);  // complemento do ratio

        float* ret =  new float[N_INTERPIE];
        ret[YCREEN] = alpha*v2[YCREEN] + beta*v1[YCREEN];
        ret[XCREEN] = alpha*v2[XCREEN] + beta*v1[XCREEN];    //interpolaão dos atributos
        ret[WCREEN] = alpha*v2[WCREEN] + beta*v1[WCREEN];
        ret[ZNDC] = alpha*v2[ZNDC] + beta*v1[ZNDC];


        ret[ONE] = alpha*v2[ONE] + beta*v1[ONE];

        ret[RCOL] = alpha*v2[RCOL] + beta*v1[RCOL];
        ret[GCOL] = alpha*v2[GCOL] + beta*v1[GCOL];
        ret[BCOL] = alpha*v2[BCOL] + beta*v1[BCOL];

        ret[XNORMAL] = alpha*v2[XNORMAL] + beta*v1[XNORMAL];
        ret[YNORMAL] = alpha*v2[YNORMAL] + beta*v1[YNORMAL];
        ret[ZNORMAL] = alpha*v2[ZNORMAL] + beta*v1[ZNORMAL];

        ret[XEYE] = alpha*v2[XEYE] + beta*v1[XEYE];
        ret[YEYE] = alpha*v2[YEYE] + beta*v1[YEYE];
        ret[ZEYE] = alpha*v2[ZEYE] + beta*v1[ZEYE];

        ret[STEXT] = alpha*v2[STEXT] + beta*v1[STEXT];
        ret[TTEXT] = alpha*v2[TTEXT] + beta*v1[TTEXT];

        return ret;
    }


    void drawPoint(float* v1){
        //fazemos a segunda divisão por w (1/w)
        this->deltas /= v1[ONE];
        this->deltat /= v1[ONE];
        wtildivide(v1);
        //realizamos o depth testing
        if(v1[ZNDC]>=depth_buffer[(((int)round(v1[YCREEN]) * screen_width) +(int)round(v1[XCREEN]))]){
            //printf("OPAOPAOPAOPAO\n");
            return;
        }


        //caso precisamos calcular a cor de um fragmento em específico
        if((shading_index==3) || apply_text){//faz phong OVERHEAD? SIM SENHOR
            glm::vec4 position_eye = glm::vec4(v1[XEYE], v1[YEYE], v1[ZEYE], 1.0f);
            glm::vec4 normalP = glm::vec4(v1[XNORMAL], v1[YNORMAL], v1[ZNORMAL], 0.0f);
            //printf("\nNormal Tri %d Vert %d: %f\t%f\t%f",t,v, normaG.x, normaG.y, normaG.z);
            glm::vec3 color;
            if(apply_text) color = text_mapper->text_mapping(position_eye, normalP, v1[STEXT], v1[TTEXT],
                                                             this->deltas, this->deltat);
            else color = phongouraud(position_eye, normalP);
            v1[RCOL] = color.r;
            v1[GCOL] = color.g;
            v1[BCOL] = color.b;
        }

        //finalmente desenhamos nosso frsgmento no color buffer
        color_buffer[(((int)round(v1[YCREEN]) * screen_width) +(int)round(v1[XCREEN]))  * 4 +0] = v1[RCOL];
        color_buffer[(((int)round(v1[YCREEN]) * screen_width) +(int)round(v1[XCREEN])) * 4 +1] = v1[GCOL];
        color_buffer[(((int)round(v1[YCREEN]) * screen_width) +(int)round(v1[XCREEN]))  * 4 +2] = v1[BCOL];
        //e atualizamos o depth buffer
        depth_buffer[(((int)round(v1[YCREEN]) * screen_width) +(int)round(v1[XCREEN]))]=v1[ZNDC];
    }

    //debugging
    void printcolor(float* v, char* str){
        printf("\n%s %f\t%f\t%f",str, v[RCOL], v[GCOL], v[BCOL]);
    }


};

