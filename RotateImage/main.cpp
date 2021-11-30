#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <io.h>
#include <png.h>


using namespace std;

void getAllFiles(string path, vector<string>& files, string fileType, vector<string>& fileName);
void rotateImage(string path, string name);


string FilePath = "input";                 //�����ļ�Ŀ¼
string OutPath = "output\\";               //���Ŀ¼
string FileTail = ".png";                  //Ŀ���ļ���׺



void thread_function() {
    printf("��һ����ȡͼƬ������Ϣ\n");
    printf("�ڶ�����������Ϣ����洢\n");
    printf("������������ͼƬ��д��������Ϣ\n");
}

int main(int argc, char** argv)
{
    //�������߳�
    thread threadObj(thread_function);

    vector<string> temp;
    vector<string> name;
    getAllFiles(FilePath, temp, FileTail, name);
    for (int i = 0; i < temp.size(); ++i)
    {
        rotateImage(temp[i], name[i]);
    }
    threadObj.join();
    return 0;
}


//��ȡĿ¼�������ļ���·�������ж��Ƿ�Ϸ�
void getAllFiles(string path, vector<string>& files, string fileType, vector<string>& fileName)
{
    // �ļ����
    long hFile = 0;
    // �ļ���Ϣ
    struct _finddata_t fileinfo;

    string p;

    if ((hFile = _findfirst(p.assign(path).append("\\*" + fileType).c_str(), &fileinfo)) != -1) {
        do {
            //�����ļ���ȫ·��
            files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            fileName.push_back(fileinfo.name);

        } while (_findnext(hFile, &fileinfo) == 0); //Ѱ����һ�����ɹ�����0������-1

        _findclose(hFile);
    }
}


//��תͼƬ
//��һ����ȡͼƬ������Ϣ
//�ڶ�����������Ϣ����洢
//������������ͼƬ��д��������Ϣ
void rotateImage(string path, string name)
{
    int width, height;

    FILE* fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;
    unsigned char* buff;

    int x, y;

    fp = fopen(path.c_str(), "rb");
    if (fp == NULL) {
        printf("ͼƬ��ȡʧ��");
        return;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    //��λ�ļ�ָ��
    rewind(fp);
    //��ʼ���ļ�
    png_init_io(png_ptr, fp);
    //��ȡPNGͼƬ��Ϣ����������
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL);

    //��ÿ��
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    //��ȡͼ����������������ݣ�row_pointers��߾���rgba����
    row_pointers = png_get_rows(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);        //���ͼƬ��ɫ����
    int block_size = (color_type == 6 ? 4 : 3);

    //(A)RGB
    int file_size = (width) * (height) * 4;
    buff = new unsigned char[file_size];

    int pos = 0;
    int rot, pre;
    for (x = 0; x < height; x++) {
        for (y = 0; y < width * block_size; y += block_size)
        {
            pre = x * width * block_size;
            rot = pre + (width - 1) * block_size - (pos - pre);
            buff[rot + 0] = row_pointers[x][y + 0];      //R
            buff[rot + 1] = row_pointers[x][y + 1];      //G
            buff[rot + 2] = row_pointers[x][y + 2];      //B
            if (color_type == 6)
                buff[rot + 3] = row_pointers[x][y + 3];   //A
            else
                buff[rot + 3] = 0xff;    //û��͸��ͨ�����ݣ�����䲻͸��0xff
            pos += 4;
        }
    }
    //�����Ѿ����������ݷ�תȡ��������buff

    fclose(fp);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    //������png�ļ�
    ofstream fout(OutPath+name);
    if (fout)                     //��������ɹ�
    {
        unsigned bit_depth = 8;
        unsigned pixel_byte = 4;
        unsigned row_byte = width * pixel_byte;

        FILE* fpp = fopen((OutPath + name).c_str(), "wb");
        if (fpp == NULL) {
            printf("ͼƬд��ʧ��");
            return;
        }

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info_ptr = png_create_info_struct(png_ptr);

        // ��������Դ��png ��Ҫд����ļ�·��
        png_init_io(png_ptr, fpp);
        // ���� infop ��ز�����Ҫ���ɵ�ͼƬ�ļ������Ϣ
        png_set_IHDR(png_ptr,
            info_ptr,
            width,
            height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

        // д��ͼƬ��Ϣ
        png_write_info(png_ptr, info_ptr);

        //��ȡ��ָ��
        png_bytepp row = (png_bytep*)malloc(height * sizeof(png_bytep));

        for (unsigned x = 0; x < height; ++x)
        {//����һ��
            row[x] = (png_bytep)malloc(row_byte);
            for (unsigned y = 0; y < row_byte; y += pixel_byte)
            {
                row[x][y + 0] = buff[x * row_byte + y + 0];
                row[x][y + 1] = buff[x * row_byte + y + 1];
                row[x][y + 2] = buff[x * row_byte + y + 2];
                row[x][y + 3] = buff[x * row_byte + y + 3];
            }
        }

        // д��ͼƬ��������
        png_write_image(png_ptr, row);
        // ����д��
        png_write_end(png_ptr, NULL);

        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

        fclose(fpp);
    }else
    {
        printf("ͼƬ����ʧ��");
        return;
    }
}


