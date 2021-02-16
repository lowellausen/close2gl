#ifndef TEXT_MAPS_C
#define TEXT_MAPS_C

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

extern int filter_mode;
extern bool apply_text;
extern glm::vec3 model_color;
extern int shading_index;

#define NN 0
#define BILI 1
#define MM 2


class MyMapeur
{
public:
    unsigned char * imagedata;
    int height;
    int  width;
    int nmups;

    MyMapeur* mipsmaps;


    MyMapeur(unsigned char *imagedata, int width, int height){
        this->imagedata = imagedata;
        this->width = width;
        this->height = height;
        this->nmups = log2(height) +1;

        this->mipsmaps = new MyMapeur[nmups];
    }

    MyMapeur(){

    }

    void setall(unsigned char *imagedata, int width, int height){
        this->imagedata = imagedata;
        this->width = width;
        this->height = height;
        this->nmups = log2(height) +1;

        this->mipsmaps = new MyMapeur[nmups];
    }


    glm::vec3 text_mapping(glm::vec4 position_eye,glm::vec4 normal,float s,float t, float deltas, float deltat){
        //apenas chama o método de filtro correto
        glm::vec3 colorT;

        //printf("\ns: %f\t t: %f", s, t);

        if(s<0.0f)s=0.0f;
        if(s>1.0f)s=1.0f;
        if(t<0.0f)t=0.0f;
        if(t>1.0f)t=1.0f;


        switch(filter_mode){
                case NN:
                colorT = nearest_neighbours(position_eye, normal, s, t); break;
                case BILI:
                colorT = bilinear(position_eye, normal, s, t); break;
                case MM:
                colorT = mipmap(position_eye, normal, s, t, deltas, deltat); break;
            }

        if(shading_index == 3)colorT = phongs(colorT, position_eye, normal);

        return colorT;

    }

    glm::vec3 nearest_neighbours(glm::vec4 position_eye,glm::vec4 normal, float s, float t){
        s = (this->width-1) * s;
        t = (this->height-1) * t;




        unsigned char* cur_texel = this->imagedata + ((int)std::round(s) + this->height * (int)std::round(t))*3;
        unsigned char r = (float)cur_texel[0];
        unsigned char g = (float)cur_texel[1];
        unsigned char b = (float)cur_texel[2];

        //printf("\nr: %f\t g: %f\tb: %f", r/255.0f ,g/255.0f,b/255.0f);

        return glm::vec3(r/255.0f, g/255.0f, b/255.0f);


    }

    glm::vec3 bilinear(glm::vec4 position_eye,glm::vec4 normal, float s, float t){
        s = (this->width-1) * s;
        t = (this->height-1) * t;

        int floorS = (int)std::floor(s);
        int floorT = (int)std::floor(t);

        float salpha = s - floorS;
        float talpha = t - floorT;

        unsigned char* texUL = this->imagedata + (floorS + this->height * (floorT+1))*3; //Upper Left texel
        unsigned char rUL = texUL[0];
        unsigned char gUL = texUL[1];
        unsigned char bUL = texUL[2];

        unsigned char* texUR = this->imagedata + ((floorS+1) + this->height * (floorT+1))*3; //Upper Right texel
        unsigned char rUR = texUR[0];
        unsigned char gUR = texUR[1];
        unsigned char bUR = texUR[2];

        unsigned char* texBL = this->imagedata + (floorS + this->height * floorT)*3; //Bottom Left texel
        unsigned char rBL = texBL[0];
        unsigned char gBL = texBL[1];
        unsigned char bBL = texBL[2];

        unsigned char* texBR = this->imagedata + ((floorS+1) + this->height * floorT)*3; //Bottom Right texel
        unsigned char rBR = texBR[0];
        unsigned char gBR = texBR[1];
        unsigned char bBR = texBR[2];



        float r =  ((1-salpha)*((1-talpha)*rBL + talpha*rUL))+(salpha*((1-talpha)*rBR + talpha*rUR));
        float g =  ((1-salpha)*((1-talpha)*gBL + talpha*gUL))+(salpha*((1-talpha)*gBR + talpha*gUR));
        float b =  ((1-salpha)*((1-talpha)*bBL + talpha*bUL))+(salpha*((1-talpha)*bBR + talpha*bUR));
        //printf("\nr: %d\t g: %d\tb: %d", r ,g,b);

        return glm::vec3(r/256.0f, g/256.0f, b/256.0f);
    }

    glm::vec3 mipmap(glm::vec4 position_eye,glm::vec4 normal, float s, float t, float ds, float dt){
        float miplevel = (float) log2(std::max((ds*this->width), (dt*this->height))); //calcula nivel de mipmap

        int lowerMep = std::floor(miplevel);
        float malpha = miplevel - lowerMep;
        int upperMop = lowerMep + 1;

        //printf("\nDeltaS: %f\tDeltaT: %f", ds, dt);
        //printf("\nMipLevel: %f\tLowerMips: %d\tUpperMips: %d",miplevel, lowerMep, upperMop);

        glm::vec3 upperColour;
        glm::vec3 lowerColour;

        //pega as cores dos dois levels de mipmaps adjacenes usando sampling bilinear
        if(lowerMep < 0) lowerMep = 0;
        if(upperMop < 0) upperMop = 0;
        if(upperMop >= nmups) upperMop = nmups - 1;
        if(lowerMep >= nmups) lowerMep = nmups - 1;

        upperColour = this->mipsmaps[upperMop].bilinear(position_eye, normal, s, t);
        lowerColour = this->mipsmaps[lowerMep].bilinear(position_eye, normal, s, t);

        //interpola linearmente as duas interpolações bilineares
        return (1 - malpha)*lowerColour + malpha*upperColour;
    }

    void init_mipsmaps(){
        if(this->width != this->height)exit(123); //tenho que considerar caso não quadrado?? depois!

        int m =0; //índex de mopsmups

        //primeiro mipmap é a imagem original

        this->mipsmaps[m].setall(this->imagedata,  this->height,  this->height);

        m++;


        for(int cheight = this->height/2; cheight>=1; cheight /= 2){//fazemos mapmeps log height vezes
            //fprintf(stderr,"\nm: %d",m);

            unsigned char * cimage = get_mops(m, cheight);

            //fprintf(stderr,"\nm: %d",m);

            this->mipsmaps[m].setall(cimage, cheight, cheight);


            m++;
        }
    }

    unsigned char * get_mops(int m, int theight){
        unsigned char * nimage = new unsigned char [theight*theight*4]; //imagem de retorno new image
        unsigned char * pimage = this->mipsmaps[m-1].imagedata; //mipmap anterior prev image

        for(int x =0; x<theight; x++){
            for(int y=0; y<theight; y++){

                //fprintf(stderr, "\nx: %d\ty:%d",x,y);
                glm::vec3 t1 = get_color(pimage,theight*2,x*2,y*2);
                glm::vec3 t2 = get_color(pimage,theight*2,(x*2)+1,y*2);
                glm::vec3 t3 = get_color(pimage,theight*2,x*2,(y*2)+1);
                glm::vec3 t4 = get_color(pimage,theight*2,(x*2)+1,(y*2)+1);

                glm::vec3 tmean = glm::vec3((t1.r+t2.r+t3.r+t4.r)/4,
                                            (t1.g+t2.g+t3.g+t4.g)/4,
                                            (t1.b+t2.b+t3.b+t4.b)/4);

                unsigned char* mutex = nimage + (x + theight * y)*3;

                mutex[0] = static_cast<unsigned char>(tmean.r);
                mutex[1] = static_cast<unsigned char>(tmean.g);
                mutex[2] = static_cast<unsigned char>(tmean.b);
            }
        }
        return nimage;
    }

    static glm::vec3 get_color(unsigned char * image,int height,int s,int t){
        unsigned char* mutex = image + (s + height * t)*3;

        return glm::vec3(mutex[0], mutex[1], mutex[2]);
    }


    glm::vec3 phongs(glm::vec3 color, glm::vec4 position_eye, glm::vec4 normal){

                //calculo de ilu que serve tanto para gouraud ads quanto para phong
        //para gouraud é chamada uma vez para cada vértice
        //em phong vai ser chamado uma vez em cada fragmento

        //todos os coeficientes são a cord do modelo
        //glm::vec3 Kd = model_color;
        glm::vec3 Ks = model_color;
        //glm::vec3 Ka = model_color;
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
        return (color * ambient_light_spectrum + color * light_spectrum * diffuse + Ks * light_spectrum * specular);

    }



};


#endif
