#ifndef MATRIX_C
#define MATRIX_C

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <time.h>
#include <glm/gtc/type_ptr.hpp>

class MyMate
{
public:
    float mat[16]; //minha matriz _quatro_ por _quatro_

    MyMate(){
        float m[16] = {1,0,0,0,  0,1,0,0,    0,0,1,0,    0,0,0,1};

        std::memcpy(this->mat, m, 16*sizeof(float));
    }


    void view_matrix(glm::vec3 camera_center, glm::vec3 view_vector, glm::vec3 up_vector){
        glm::vec3 n = -view_vector; //camera olha para z negativo
        n = normalize(n);
        glm::vec3 u = normalize(crossproduct(normalize(up_vector),n));
        glm::vec3 v = crossproduct(n,u);

        glm::vec3 c_vec = -camera_center; //vetor do centro da câmera para a origem

        float m[16] =
        {u.x   , u.y   , u.z   , dotproduct(u , c_vec),
         v.x   , v.y   , v.z   , dotproduct(v , c_vec),
         n.x   , n.y   , n.z   , dotproduct(n , c_vec),
         0.0f  , 0.0f  , 0.0f  ,      1.0f
        };

        std::memcpy(this->mat, m, 16*sizeof(float));
    }


    void projection_matrix(float field_of_view, float aspect, float n, float f)
    {
        //tan(glm::radians(fovY / 2)) / tan(glm::radians(fovX / 2));

        float t = fabs(n) * tan(field_of_view / 2.0f);
        //float t = fabs(n) *tan(glm::radians(vfov / 2)) / tan(glm::radians(hfov / 2));
        float b = -t;
        float r = t * aspect;
        float l = -r;


        float m[16] =
        {(2*n)/(r-l),   0.0f,   (r+l)/(r-l),    0.0f,
         0.0f,      (2*n)/(t-b),(t+b)/(t-b),    0.0f,
         0.0f,          0.0f,  -(f+n)/(f-n),-(2*f*n)/(f-n),
         0.0f,          0.0f,       -1.0f  ,    0.0f};

        std::memcpy(this->mat, m, 16*sizeof(float));
    }

    void viewport_matrix(int x, int y, int width, int height){

        float m[16] =
        {width/2.0f,   0.0f,   0.0f,    x+(width/2.0f),
         0.0f,   -height/2.0f,    0.0f,    y+(height/2.0f),
         0.0f,          0.0f,  1.0f,       0.0f,         //maxZ-minZ, minZ ??
         0.0f,          0.0f,       0.0f  ,    1.0f};

        std::memcpy(this->mat, m, 16*sizeof(float));
    }

    void rotation_matrix(glm::vec3 axis, float ang)
    {
       float c = cos(ang);
       float s = sin(ang);
       float x = axis.x;
       float y = axis.y;
       float z = axis.z;

       float m[16]= {
           x*x*(1-c)+c  , x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0.0f,
           x*y*(1-c)+z*s, y*y*(1-c)+c  , y*z*(1-c)-x*s, 0.0f,
           x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z*(1-c)+c ,  0.0f,
           0.0f         , 0.0f         , 0.0f        ,  1.0f};

       std::memcpy(this->mat, m, 16*sizeof(float));
    }


    void matrix_rotate(glm::vec3 axis, float ang)
    {
       MyMate* rodrigues = new MyMate();

       float c = cos(ang);
       float s = sin(ang);
       float x = axis.x;
       float y = axis.y;
       float z = axis.z;

       float m[16]= {
           x*x*(1-c)+c  , x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0.0f,
           x*y*(1-c)+z*s, y*y*(1-c)+c  , y*z*(1-c)-x*s, 0.0f,
           x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z*(1-c)+c ,  0.0f,
           0.0f         , 0.0f         , 0.0f        ,  1.0f};

       std::memcpy(rodrigues->mat, m, 16*sizeof(float));

       this->mult(rodrigues);
    }


    void mult(MyMate* mat_r){
        float mul [16], mat[16];
        std::memcpy(mat, mat_r->mat, 16*sizeof(float));

        mul[0]=this->mat[0]*mat[0]+this->mat[1]*mat[4]+this->mat[2]*mat[8]+this->mat[3]*mat[12];
        mul[1]=this->mat[0]*mat[1]+this->mat[1]*mat[5]+this->mat[2]*mat[9]+this->mat[3]*mat[13];
        mul[2]=this->mat[0]*mat[2]+this->mat[1]*mat[6]+this->mat[2]*mat[10]+this->mat[3]*mat[14];
        mul[3]=this->mat[0]*mat[3]+this->mat[1]*mat[7]+this->mat[2]*mat[11]+this->mat[3]*mat[15];
        mul[4]=this->mat[4]*mat[0]+this->mat[5]*mat[4]+this->mat[6]*mat[8]+this->mat[7]*mat[12];
        mul[5]=this->mat[4]*mat[1]+this->mat[5]*mat[5]+this->mat[6]*mat[9]+this->mat[7]*mat[13];
        mul[6]=this->mat[4]*mat[2]+this->mat[5]*mat[6]+this->mat[6]*mat[10]+this->mat[7]*mat[14];
        mul[7]=this->mat[4]*mat[3]+this->mat[5]*mat[7]+this->mat[6]*mat[11]+this->mat[7]*mat[15];
        mul[8]=this->mat[8]*mat[0]+this->mat[9]*mat[4]+this->mat[10]*mat[8]+this->mat[11]*mat[12];
        mul[9]=this->mat[8]*mat[1]+this->mat[9]*mat[5]+this->mat[10]*mat[9]+this->mat[11]*mat[13];
        mul[10]=this->mat[8]*mat[2]+this->mat[9]*mat[6]+this->mat[10]*mat[10]+this->mat[11]*mat[14];
        mul[11]=this->mat[8]*mat[3]+this->mat[9]*mat[7]+this->mat[10]*mat[11]+this->mat[11]*mat[15];
        mul[12]=this->mat[12]*mat[0]+this->mat[13]*mat[4]+this->mat[14]*mat[8]+this->mat[15]*mat[12];
        mul[13]=this->mat[12]*mat[1]+this->mat[13]*mat[5]+this->mat[14]*mat[9]+this->mat[15]*mat[13];
        mul[14]=this->mat[12]*mat[2]+this->mat[13]*mat[6]+this->mat[14]*mat[10]+this->mat[15]*mat[14];
        mul[15]=this->mat[12]*mat[3]+this->mat[13]*mat[7]+this->mat[14]*mat[11]+this->mat[15]*mat[15];

        std::memcpy(this->mat, mul, 16*sizeof(float));
    }

    glm::vec4 mult(glm::vec4 vecin){
        glm::vec4 vecout;

        vecout[0] = this->mat[0]*vecin[0]+this->mat[1]*vecin[1]+this->mat[2]*vecin[2]+this->mat[3]*vecin[3];
        vecout[1] = this->mat[4]*vecin[0]+this->mat[5]*vecin[1]+this->mat[6]*vecin[2]+this->mat[7]*vecin[3];
        vecout[2] = this->mat[8]*vecin[0]+this->mat[9]*vecin[1]+this->mat[10]*vecin[2]+this->mat[11]*vecin[3];
        vecout[3] = this->mat[12]*vecin[0]+this->mat[13]*vecin[1]+this->mat[14]*vecin[2]+this->mat[15]*vecin[3];

        return vecout;
    }

    void test(){/*
        MyMate* mat= new MyMate();
        float m[16] =  {1,2,3,4,  5,6,7,8,    9,10,11,12,    13,14,15,16};
        std::memcpy(mat->mat, m, 16*sizeof(float));
        std::memcpy(this->mat, m, 16*sizeof(float));



        clock_t begin = clock();
        for (int i = 0; i<10000; i++)
        mult(mat);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        std::cout << time_spent<<"\n";

        glm::mat4 bbb;
        memcpy( glm::value_ptr( bbb ), m, sizeof( m ) );
        glm::mat4 aaa;
        memcpy( glm::value_ptr( aaa ), m, sizeof( m ) );

        clock_t begin2 = clock();
        for (int i = 0; i<10000; i++)
        aaa*bbb;
        clock_t end2 = clock();
        double time_spent2 = (double)(end2 - begin2) / CLOCKS_PER_SEC;

        std::cout << time_spent2 << "\n";*/

        print();

    }

    void print(){
        std::cout <<  this->mat[0] << " " <<  this->mat[1]<< " " <<  this->mat[2]<< " " <<  this->mat[3] << "\n";
        std::cout <<  this->mat[4] << " "  << this->mat[5] << " "<<  this->mat[6] << " "<<  this->mat[7] << "\n";
        std::cout <<  this->mat[8] << " "  <<this->mat[9]<< " " <<  this->mat[10]<< " " <<  this->mat[11] << "\n";
        std::cout <<  this->mat[12]<< " " <<  this->mat[13]<< " " <<  this->mat[14]<< " " <<  this->mat[15] << "\n";
    }

    static float norm(glm::vec3 vec){return sqrt( vec.x*vec.x + vec.y*vec.y + vec.z*vec.z );}

    static glm::vec3 normalize(glm::vec3 vec){float n = norm(vec);return glm::vec3(vec[0]/n,vec[1]/n,vec[2]/n);}

    static glm::vec3 crossproduct(glm::vec3 u, glm::vec3 v){return glm::vec3(u.y*v.z - u.z*v.y,u.z*v.x - u.x*v.z,u.x*v.y - u.y*v.x);}

    static float dotproduct(glm::vec3 u, glm::vec3 v){return u.x*v.x + u.y*v.y + u.z*v.z;}

    static void printmat4(glm::mat4 M)
    {
        printf("\n");
        printf("[ %+0.2f  %+0.2f  %+0.2f  %+0.2f ]\n", M[0][0], M[1][0], M[2][0], M[3][0]);
        printf("[ %+0.2f  %+0.2f  %+0.2f  %+0.2f ]\n", M[0][1], M[1][1], M[2][1], M[3][1]);
        printf("[ %+0.2f  %+0.2f  %+0.2f  %+0.2f ]\n", M[0][2], M[1][2], M[2][2], M[3][2]);
        printf("[ %+0.2f  %+0.2f  %+0.2f  %+0.2f ]\n", M[0][3], M[1][3], M[2][3], M[3][3]);
    }


};

#endif
