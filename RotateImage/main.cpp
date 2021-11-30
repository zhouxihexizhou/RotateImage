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


string FilePath = "input";                 //输入文件目录
string OutPath = "output\\";               //输出目录
string FileTail = ".png";                  //目标文件后缀



void thread_function() {
    printf("第一步读取图片像素信息\n");
    printf("第二步将像素信息反向存储\n");
    printf("第三步生成新图片，写入像素信息\n");
}

int main(int argc, char** argv)
{
    //开启新线程
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


//获取目录下所有文件的路径，并判断是否合法
void getAllFiles(string path, vector<string>& files, string fileType, vector<string>& fileName)
{
    // 文件句柄
    long hFile = 0;
    // 文件信息
    struct _finddata_t fileinfo;

    string p;

    if ((hFile = _findfirst(p.assign(path).append("\\*" + fileType).c_str(), &fileinfo)) != -1) {
        do {
            //保存文件的全路径
            files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            fileName.push_back(fileinfo.name);

        } while (_findnext(hFile, &fileinfo) == 0); //寻找下一个，成功返回0，否则-1

        _findclose(hFile);
    }
}


//旋转图片
//第一步读取图片像素信息
//第二步将像素信息反向存储
//第三步生成新图片，写入像素信息
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
        printf("图片读取失败");
        return;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    //复位文件指针
    rewind(fp);
    //开始读文件
    png_init_io(png_ptr, fp);
    //读取PNG图片信息和像素数据
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL);

    //获得宽高
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    //获取图像的所有行像素数据，row_pointers里边就是rgba数据
    row_pointers = png_get_rows(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);        //获得图片颜色类型
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
                buff[rot + 3] = 0xff;    //没有透明通道数据，则填充不透明0xff
            pos += 4;
        }
    }
    //这里已经将像素数据翻转取出，存入buff

    fclose(fp);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    //创建新png文件
    ofstream fout(OutPath+name);
    if (fout)                     //如果创建成功
    {
        unsigned bit_depth = 8;
        unsigned pixel_byte = 4;
        unsigned row_byte = width * pixel_byte;

        FILE* fpp = fopen((OutPath + name).c_str(), "wb");
        if (fpp == NULL) {
            printf("图片写入失败");
            return;
        }

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info_ptr = png_create_info_struct(png_ptr);

        // 关联数据源，png 和要写入的文件路径
        png_init_io(png_ptr, fpp);
        // 设置 infop 相关参数，要生成的图片文件相关信息
        png_set_IHDR(png_ptr,
            info_ptr,
            width,
            height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

        // 写入图片信息
        png_write_info(png_ptr, info_ptr);

        //获取行指针
        png_bytepp row = (png_bytep*)malloc(height * sizeof(png_bytep));

        for (unsigned x = 0; x < height; ++x)
        {//分配一行
            row[x] = (png_bytep)malloc(row_byte);
            for (unsigned y = 0; y < row_byte; y += pixel_byte)
            {
                row[x][y + 0] = buff[x * row_byte + y + 0];
                row[x][y + 1] = buff[x * row_byte + y + 1];
                row[x][y + 2] = buff[x * row_byte + y + 2];
                row[x][y + 3] = buff[x * row_byte + y + 3];
            }
        }

        // 写入图片像素内容
        png_write_image(png_ptr, row);
        // 结束写入
        png_write_end(png_ptr, NULL);

        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

        fclose(fpp);
    }else
    {
        printf("图片创建失败");
        return;
    }
}


