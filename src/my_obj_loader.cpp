#include <iostream>
#include <glm\vec3.hpp>
#include<string.h>


#define MAX_MATERIAL_COUNT 1


class MyObj
{
public:
    bool has_text = false;
	int NumTris;
	float* Vert;
	float* Vert_Normal;
	float* Text_coord;
	struct Triangle {
		glm::vec3 v0, v1, v2, Norm[3], face_normal;
		float s[3], t[3];
		unsigned char Color[3];
	};
	glm::vec3 bbox_min = glm::vec3();
	glm::vec3 bbox_max = glm::vec3();
	glm::vec3 bbox_middle = glm::vec3(0.0f);



	void Load(char* FileName){

        glm::vec3 ambient[MAX_MATERIAL_COUNT];
        glm::vec3 diffuse[MAX_MATERIAL_COUNT];
        glm::vec3 specular[MAX_MATERIAL_COUNT];

		float shine[MAX_MATERIAL_COUNT];

		int material_count, color_index[3], i;

		char  ch;
		char *tex;

		FILE* fp = fopen(FileName, "r");
		if (fp == NULL) {
			printf("ERROR: unable to open TriObj [%s]!\n", FileName); exit(1);
		}

		fscanf(fp, "%c", &ch);
		while (ch != '\n')// skip the first line – object’s name
			fscanf(fp, "%c", &ch);
		//
		fscanf(fp, "# triangles = %d\n", &NumTris);// read # of triangles
		fscanf(fp, "Material count = %d\n", &material_count);// read material count
		//
		for (i = 0; i<material_count; i++) {
			fscanf(fp, "ambient color %f %f %f\n", &(ambient[i].x), &(ambient[i].y), &(ambient[i].z));
			fscanf(fp, "diffuse color %f %f %f\n", &(diffuse[i].x), &(diffuse[i].y), &(diffuse[i].z));
			fscanf(fp, "specular color %f %f %f\n", &(specular[i].x),&(specular[i].y), &(specular[i].z));
			fscanf(fp, "material shine %f\n", &(shine[i]));
			fscanf(fp, "Texture = %s\n", tex);
			//printf("\n%s\n", tex);
			if(!strcmp(tex,"YES"))this->has_text = true;
		}
		//
		fscanf(fp, "%c", &ch);
		while (ch != '\n')// skip documentation line
			fscanf(fp, "%c", &ch);
		//
		//  allocate triangles for tri model
		//
		printf("Reading in %s (%d triangles). . .\n", FileName, NumTris);
		Triangle* Tris = new Triangle[NumTris];
		//
		if(!this->has_text){
		for (i = 0; i<NumTris; i++) // read triangles
		{
			fscanf(fp, "v0 %f %f %f %f %f %f %d\n",
				&(Tris[i].v0.x), &(Tris[i].v0.y), &(Tris[i].v0.z),
				&(Tris[i].Norm[0].x), &(Tris[i].Norm[0].y), &(Tris[i].Norm[0].z),
				&(color_index[0]));
			fscanf(fp, "v1 %f %f %f %f %f %f %d\n",
				&(Tris[i].v1.x), &(Tris[i].v1.y), &(Tris[i].v1.z),
				&(Tris[i].Norm[1].x), &(Tris[i].Norm[1].y), &(Tris[i].Norm[1].z),
				&(color_index[1]));
			fscanf(fp, "v2 %f %f %f %f %f %f %d\n",
				&(Tris[i].v2.x), &(Tris[i].v2.y), &(Tris[i].v2.z),
				&(Tris[i].Norm[2].x), &(Tris[i].Norm[2].y), &(Tris[i].Norm[2].z),
				&(color_index[2]));
			fscanf(fp, "face normal %f %f %f\n", &(Tris[i].face_normal.x),
				&(Tris[i].face_normal.y),
				&(Tris[i].face_normal.z));
			//
			Tris[i].Color[0] = (unsigned char)(int)(255 * (diffuse[color_index[0]].x));
			Tris[i].Color[1] = (unsigned char)(int)(255 * (diffuse[color_index[0]].y));
			Tris[i].Color[2] = (unsigned char)(int)(255 * (diffuse[color_index[0]].z));
		}}
		else{
        for (i = 0; i<NumTris; i++) // read triangles
        {
            fscanf(fp, "v0 %f %f %f %f %f %f %d %f %f\n",
                &(Tris[i].v0.x), &(Tris[i].v0.y), &(Tris[i].v0.z),
                &(Tris[i].Norm[0].x), &(Tris[i].Norm[0].y), &(Tris[i].Norm[0].z),
                &(color_index[0]), &(Tris[i].s[0]), &(Tris[i].t[0]));
            fscanf(fp, "v1 %f %f %f %f %f %f %d %f %f\n",
                &(Tris[i].v1.x), &(Tris[i].v1.y), &(Tris[i].v1.z),
                &(Tris[i].Norm[1].x), &(Tris[i].Norm[1].y), &(Tris[i].Norm[1].z),
                &(color_index[1]), &(Tris[i].s[1]), &(Tris[i].t[1]));
            fscanf(fp, "v2 %f %f %f %f %f %f %d %f %f\n",
                &(Tris[i].v2.x), &(Tris[i].v2.y), &(Tris[i].v2.z),
                &(Tris[i].Norm[2].x), &(Tris[i].Norm[2].y), &(Tris[i].Norm[2].z),
                &(color_index[2]), &(Tris[i].s[2]), &(Tris[i].t[2]));
            fscanf(fp, "face normal %f %f %f\n", &(Tris[i].face_normal.x),
                &(Tris[i].face_normal.y),
                &(Tris[i].face_normal.z));
            //
            Tris[i].Color[0] = (unsigned char)(int)(255 * (diffuse[color_index[0]].x));
            Tris[i].Color[1] = (unsigned char)(int)(255 * (diffuse[color_index[0]].y));
            Tris[i].Color[2] = (unsigned char)(int)(255 * (diffuse[color_index[0]].z));
        }
		}
		fclose(fp);
		//
        //     For use in the vertex buffer objects in your application, pack the vertex and normal data
        //           into vectors
		//
		Vert = new float[9 * NumTris];
		Vert_Normal = new float[9 * NumTris];

		this->Text_coord = new float[6 * NumTris];

		bbox_min = Tris[0].v0;
		bbox_max = Tris[0].v0;

		for (i = 0; i<NumTris; i++) {
			//    vertex coordinates
			Vert[9 * i] = Tris[i].v0.x;
			if(bbox_max.x<Vert[9 * i])bbox_max.x=Vert[9 * i];
			if(bbox_min.x>Vert[9 * i])bbox_min.x=Vert[9 * i];
			Vert[9 * i + 1] = Tris[i].v0.y;
			if(bbox_max.y<Vert[9 * i + 1])bbox_max.y=Vert[9 * i +1];
			if(bbox_min.y>Vert[9 * i + 1])bbox_min.y=Vert[9 * i +1];
			Vert[9 * i + 2] = Tris[i].v0.z;
			if(bbox_max.z<Vert[9 * i + 2])bbox_max.z=Vert[9 * i + 2];
			if(bbox_min.z>Vert[9 * i + 2])bbox_min.z=Vert[9 * i + 2];
			Vert[9 * i + 3] = Tris[i].v1.x;
			if(bbox_max.x<Vert[9 * i + 3])bbox_max.x=Vert[9 * i + 3];
			if(bbox_min.x>Vert[9 * i + 3])bbox_min.x=Vert[9 * i + 3];
			Vert[9 * i + 4] = Tris[i].v1.y;
			if(bbox_max.y<Vert[9 * i + 4])bbox_max.y=Vert[9 * i + 4];
			if(bbox_min.y>Vert[9 * i + 4])bbox_min.y=Vert[9 * i + 4];
			Vert[9 * i + 5] = Tris[i].v1.z;
			if(bbox_max.z<Vert[9 * i + 5])bbox_max.z=Vert[9 * i + 5];
			if(bbox_min.z>Vert[9 * i + 5])bbox_min.z=Vert[9 * i + 5];
			Vert[9 * i + 6] = Tris[i].v2.x;
			if(bbox_max.x<Vert[9 * i + 6])bbox_max.x=Vert[9 * i + 6];
			if(bbox_min.x>Vert[9 * i + 6])bbox_min.x=Vert[9 * i + 6];
			Vert[9 * i + 7] = Tris[i].v2.y;
			if(bbox_max.y<Vert[9 * i + 7])bbox_max.y=Vert[9 * i + 7];
			if(bbox_min.y>Vert[9 * i + 7])bbox_min.y=Vert[9 * i + 7];
			Vert[9 * i + 8] = Tris[i].v2.z;
			if(bbox_max.z<Vert[9 * i + 8])bbox_max.z=Vert[9 * i + 8];
			if(bbox_min.z>Vert[9 * i + 8])bbox_min.z=Vert[9 * i + 8];

			//    vertex normal coordinates
			Vert_Normal[9 * i] = Tris[i].Norm[0].x;
			Vert_Normal[9 * i + 1] = Tris[i].Norm[0].y;
			Vert_Normal[9 * i + 2] = Tris[i].Norm[0].z;
			Vert_Normal[9 * i + 3] = Tris[i].Norm[1].x;
			Vert_Normal[9 * i + 4] = Tris[i].Norm[1].y;
			Vert_Normal[9 * i + 5] = Tris[i].Norm[1].z;
			Vert_Normal[9 * i + 6] = Tris[i].Norm[2].x;
			Vert_Normal[9 * i + 7] = Tris[i].Norm[2].y;
			Vert_Normal[9 * i + 8] = Tris[i].Norm[2].z;

			if(this->has_text){
                Text_coord[6 * i + 0] = Tris[i].s[0];
                Text_coord[6 * i + 1] = Tris[i].t[0];
                Text_coord[6 * i + 2] = Tris[i].s[1];
                Text_coord[6 * i + 3] = Tris[i].t[1];
                Text_coord[6 * i + 4] = Tris[i].s[2];
                Text_coord[6 * i + 5] = Tris[i].t[2];
			}
			else{
                Text_coord[6 * i + 0] = 0.0f;
                Text_coord[6 * i + 1] = 0.0f;
                Text_coord[6 * i + 2] = 0.0f;
                Text_coord[6 * i + 3] = 0.0f;
                Text_coord[6 * i + 4] = 0.0f;
                Text_coord[6 * i + 5] = 0.0f;
			}
		}

		glm::vec3 bbox_diam = glm::vec3(bbox_max + bbox_min);
		bbox_middle = glm::vec3(bbox_diam.x/2, bbox_diam.y/2, bbox_diam.z/2);

	}




};

