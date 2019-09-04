#include "search.h"
#include "pic_handle.h"
#include "txt_orig.h"
#include "event.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <jpeglib.h>
#include <semaphore.h>



    
extern int x_tch,y_tch;     //声明外部全局变量
extern unsigned int *plcd;
extern struct set_val Set_Value;
extern struct pic_info* pic_free;




//该源文件用于进行图片像素点的获取/输出以及特效的实现
//1.Bmp_Get_Color
//2.Jpeg_Get_Color
//3.Start_Check
//4.display
//5.center_change
//6.White_Init
//7.Pic_Print
//8.Pic_Print_cirle
//9.Pic_Print_cirle_Auto
//10.Pic_Print_Cover_Left
//11.Pic_Print_Many_Wind
//12.Pic_Print_Vague_Glass_down
//13.Pic_Print_Vague_Glass_up
//14.Pic_Print_SpCover_Left
//15.Pic_Show
//16.Txt_Synth
//17.Is_fu



//该函数用于获取bmp格式图片的信息并保存
struct pic_info* Bmp_Get_Color(char *path,int flag)
{
    FILE *fp = NULL;
    fp = fopen(path,"r");//只读方式打开图片文件
    if(fp == NULL)
    {
        perror("BMP OPEN FAILED:");
        return;
    }//文件打开成功

    struct pic_info *bmp_info = malloc(sizeof(struct pic_info));//申请内存,存放颜色等信息
    pic_free = bmp_info;    //保存图片信息,用于释放资源

    memset(bmp_info->P_C,0x00ffffff,480*800*4);

    int num = 0;

    {//该代码块用于读取该bmp图片的固有属性
        fseek(fp,0x12,SEEK_SET);//偏移获取宽度
        num = fread(&bmp_info->width,4,1,fp);//宽度
        if(num == -1)
        {
            perror("WIDTH READ FAILED:");
            fclose(fp);
			return;
        }

        num = fread(&bmp_info->length,4,1,fp);//长度
        if(num == -1)
		{
			perror("LENGTH READ FAILED:");
			fclose(fp);
			return;
		}
        
        fseek(fp,0x1c,SEEK_SET);//偏移获取色深
        num = fread(&bmp_info->depth,2,1,fp);//色深
        if(num == -1)
		{
			perror("DEPTH READ FAILED:");
			fclose(fp);
			return;
		}

        //printf("x = %d y = %d d = %d\n",bmp_info->length,bmp_info->width,bmp_info->depth);//打印查看像素
    }
    
    int num_inc = 0;
    num = 0;

    if(bmp_info->depth == 24)//计算补位
    {
        int depth_temp = (3 * bmp_info->width) % 4;
        if(depth_temp == 0)
            num_inc = 0;
        else if(depth_temp == 1)
            num_inc = 3;
        else if(depth_temp == 2)
            num_inc = 2;
        else if(depth_temp == 3)
            num_inc = 1;
    }

    int length_buf = bmp_info->length * bmp_info->width * bmp_info->depth/8 + bmp_info->length * num_inc;
    //像素读取缓冲区大小,其组成为:length*width*depth/8基础像素组成点大小+length*num_inc补位数据

    char buf_color[length_buf];

    fseek(fp,0x36,SEEK_SET);
    num = fread(buf_color,1,length_buf,fp);//读取像素点
    if(num != length_buf)//获取失真
    {
        perror("BMP GAIN FAILED:");
		fclose(fp);
		return;
    }
    //像素点获取成功

    int a = 0,r = 0,g = 0,b = 0;//声明并初始化变量
    int x = 0,y = 0;
    int point_color = 0;
    num = 0;

    Start_Check(bmp_info);      //计算中心偏移量

    if(flag != TURE)
    {
        bmp_info->start_x = 0;
        bmp_info->start_y = 0;
    }

    for(y = 0;y < bmp_info->length;y++)//该循环用于合成颜色并保存
    {
        for(x = 0;x< bmp_info->width;x++)
        {
            b = buf_color[num++];
            g = buf_color[num++];
            r = buf_color[num++];
            if(bmp_info->depth == 32)
                a = buf_color[num++];
            point_color = (a << 24) | (r << 16) | (g << 8) | b;//颜色合成

            if(x >= 0 && x < 800 && y >= 0 && y < 480)      //防止图片越界
				bmp_info->P_C[bmp_info->length - y + bmp_info->start_y][x + bmp_info->start_x] = point_color;//此处颜色存储为从上至下
        }
        num += num_inc;//该行读取结束,进行补位操作
    }

    //读取完毕

    fclose(fp);     //读取完毕关闭文件
    return bmp_info;
    
}







//该函数用于获取jpeg格式图片的信息并保存
struct pic_info* Jpeg_Get_Color(char *path,int flag)
{
	//1. 分配并初始化一个jpeg解压对象
	struct jpeg_decompress_struct dinfo; //声明一个解压的对象
	
	struct jpeg_error_mgr jerr; //声明一个出错信息的对象
	
	dinfo.err = jpeg_std_error(&jerr); //初始化这个出错对象
	
	jpeg_create_decompress(&dinfo); //初始化dinfo这个解压对象

    struct pic_info* jpeg_info = malloc( sizeof(struct pic_info) );//声明并定义一个图像信息对象
    pic_free = jpeg_info;//保存图片信息,用于释放资源

	//2. 指定要解压缩的图像文件
	
		FILE *fp = NULL;
		fp = fopen(path, "r");
		if (fp == NULL)
		{
			perror("fopen error");
			return;
		}
		
		jpeg_stdio_src(&dinfo, fp); //指定要解压的图像文件
				
				
	//3.	调用jpeg_read_header()获取图像信息
		jpeg_read_header(&dinfo, TRUE);
		
	
	
	//4.	用于设置jpeg解压对象dinfo的一些参数。可采用默认参数



	/*5. 调用jpeg_start_decompress()启动解压过程
		
		
	调用jpeg_start_decompress函数之后，JPEG解压对象dinfo中
	下面这几个字段(成员变量)将会比较有用：
		dinfo.output_width: 	图像输出宽度，一行占多少个像素点
		dinfo.output_height:	图像输出高度，占多少行
		dinfo.output_components:  每个像素点的分量数，每个像素点占多少个字节
								3： R G B
								4：A R G B
				width * height * components

				
		在调用jpeg_start_decompress之后，往往需要为解压后的扫描线上的
		所有像素点分配存储空间： 
			存一行： output_width * output_components
		*/

     
        
		jpeg_start_decompress(&dinfo);      //解压准备进行像素颜色合成

        //保存图片像素信息
        jpeg_info->length = dinfo.output_height;
        jpeg_info->width = dinfo.output_width;

        printf("y = %d,x = %d\n",jpeg_info->length,jpeg_info->width);
		
	//6. 读取一行或者多行扫描线上数据并处理，通常的代码是这样子的:

		unsigned char *buffer = malloc(dinfo.output_width * dinfo.output_components);   //每次读取一行数据
	
		//dinfo.output_scanline , 表示的意思是，已经扫描了多少行
		int x,y = 0;
		while (dinfo.output_scanline < dinfo.output_height)
		{
			y = dinfo.output_scanline;//将要扫描第y行
			jpeg_read_scanlines(&dinfo,  //解压对象
								&buffer,//保存解压后数据的二级指针, 
								1 //读取多少行数据来解压
								);//该语句用于向已解压的图片数据中读取一行数据用于像素的合成操作
								//dinfo.output_scanline + 1 (该操作已在该函数内部实现)
			unsigned char *p = buffer;
			unsigned char a,r,g,b;
			int color;
			
			for(x = 0;x < dinfo.output_width;x++)
			{
				if(dinfo.output_components == 3)
				{
					a = 0;
				}
				else
				{
					a = *(p++);
				}
				r = *(p++);
				g = *(p++);
				b = *(p++);
				color = (a << 24 | r << 16 | g << 8 |b);

                if(x >= 0 && x < 800 && y >= 0 && y < 480)      //防止越界
                    jpeg_info->P_C[y][x] = color;       //保存颜色
                    //*(plcd+y*800+x) = color;
				
			}
								
			
		}
/*对扫描线的读取是按照从上到下的顺序进行的，也就是说图像最上方的扫描线最先
	被jpeg_read_scanlines()读入到存储空间中，紧接着是第二行扫描线，最后是
	图像底边的扫描线被读入到存储空间中去。*/
		

	//7. 调用jpeg_finish_decompress()完成解压过程
		jpeg_finish_decompress(&dinfo);

	//8. 调用jpeg_destroy_decompress释放jpeg解压对象dinfo
		jpeg_destroy_decompress(&dinfo);
		
		free(buffer);

        return jpeg_info;
}



//该函数用于确定居中显示时,显示起点坐标
void Start_Check(struct pic_info* pic)
{
    if(pic->length < 480)   //y像素小于屏幕
        pic->start_y = (480 - pic->length) / 2;
    else if(pic->length > 480)  //大于屏幕
        pic->start_y = 0;
    else if(pic->length == 480) //等于屏幕
        pic->start_y = 0;

    if(pic->width < 800)    //x像素小于屏幕
        pic->start_x = (800 - pic->width) / 2;
    else if(pic->width > 800)   //大于屏幕
        pic->start_x = 0;
    else if(pic->width == 800)  //等于屏幕
        pic->start_x = 0;

    return;
}



//点显示函数,支持像素点x800,y480.color格式为argb
void display(int x,int y,int color)
{
	if(x >= 0 && x < 800 && y >= 0 && y < 480)//防止越界
		*(plcd+800*y+x) = color;
	else
		return ;
}



//将图片进行居中存储的转换
void center_change(struct pic_info* pic)
{
    //暂时无法实现动态刷新,使用暂存的方法将图片进行更换,将图片进行居中存储
    int x = 0,y = 0;
    
    int buf[480][800] = {0};
    memcpy(buf,pic->P_C,480*800*4);
    
    for(y = 0;y < 480;y++)
    {
        for(x = 0;x < 800;x++)
        {
            if(x < pic->width && y < pic->length)
            {
                pic->P_C[y + pic->start_y][x + pic->start_x] = buf[y][x];
            }
        }
    }

    for(y = 0;y < 480;y++)
    {
        for(x = 0;x < 800;x++)
        {
            if(x < pic->start_x || x >= (pic->start_x + pic->width) ||
                y < pic->start_y || y >= (pic->start_y + pic->length))
            {
                pic->P_C[y][x] = 0x00ffffff;        //刷新白屏
            }
        }
    }


/*
    if(x >= 0 && x < 800 && y >= 0 && y < 480)//防止越界
    {
        //if(x < pic->start_x || x >= (pic->start_x + pic->width) ||
            //y < pic->start_y || y >= (pic->start_y + pic->length))     //超出图片范围
            //*(plcd+800*y+x) = color;       //写白屏
        //else        //图片范围内
        if(x < pic->width && y < pic->length)
        {
            *( plcd+800*(y + pic->start_y)+(x + pic->start_x) ) = color;
        }





        
            //*( plcd+800*(y + pic->start_y)+(x + pic->start_x) ) = color;
    }
*/

    
    return;
}




//屏幕刷新函数.将屏幕刷新为白色
void White_Init()
{
	int y = 0,x = 0;
	for(y = 0;y < 480;y++)		//刷新白屏
		for(x = 0;x < 800;x++)
			display(x,y,0x00ffffff);
}


//基础bmp图片输出
void Pic_Print(struct pic_info *pic)
{
    int x = 0,y = 0;
    for(y = 0;y < 480;y++)
        for(x = 0 ;x < 800;x++)
            display(x,y,pic->P_C[y][x]);
    return;
}



//点触点画圆扩散
void Pic_Print_cirle(struct pic_info *pic)
{
    int x = 0,y = 0,r = 0,len;
    int xz = 0,yz = 0;
    xz = x_tch;
    yz = y_tch;

    for(r = 0;r < 950;r++)
        for(y = 0;y < 480;y++)
            for(x = 0;x < 800;x++)
            {
                len = (xz - x)*(xz - x) + (yz - y)*(yz - y);
                if(len <= r*r)
                    display (x,y,pic->P_C[y][x]);
            }

    return;
}




void Pic_Print_cirle_Auto(struct pic_info *pic)
{
    int x = 0,y = 0,r = 0,len;
    int xz = 0,yz = 0;
    xz = rand() % pic->width;
    yz = rand() % pic->length;

    for(r = 0;r < 950;r+=3)
        for(y = 0;y < 480;y++)
            for(x = 0;x < 800;x++)
            {
                len = (xz - x)*(xz - x) + (yz - y)*(yz - y);
                if(len <= r*r)
                    display (x,y,pic->P_C[y][x]);
            }

    return;
}



//从左至右简单覆盖
void Pic_Print_Cover_Left(struct pic_info *pic)
{
    int x = 0,y = 0,loop = 0;

    //for(loop = 0; loop < bmp_info->width;loop++)
        for(x = 0; x < 800;x++)
        {
            for(y = 0;y < 480;y++)
                display (x, y, pic->P_C[y][x]);
            usleep(9999);
        }

    return;
}



//简单百叶窗
void Pic_Print_Many_Wind(struct pic_info *pic)
{
    int x = 0,y = 0,loop = 0;
    int aver = 800/6;
    int arr_aver[6];

    for(loop = 0;loop < 6;loop++)
    {
        arr_aver[loop] = aver*(loop);
    }

    aver += 60;

    for(x = 0;x < aver;x++)
    {
        for(y = 0;y < 480;y++)
        {
            for(loop = 0;loop < 6;loop++)
                display (arr_aver[loop], y, pic->P_C[y][arr_aver[loop]]);
        }

        for(loop = 0;loop < 6;loop++)
            arr_aver[loop]++;

        usleep(9999);
        
    }

    return;
}



//从下至上刷新简单毛玻璃
void Pic_Print_Vague_Glass_down(struct pic_info *pic)
{
    int x = 0,y = 0,loop = 0;

    for(y = 0;y < 480;y++)//刷新480行
        for(loop = y + 1;loop < 800;loop++)
            for(x = 0;x < 800;x++)
                display (x,loop, pic->P_C[y][x]);
            
    return;
}


//从上至下刷新简单毛玻璃
void Pic_Print_Vague_Glass_up(struct pic_info *pic)
{
    int x = 0,y = 0,loop = 0;

    printf("print start\n");

    for(y = 480;y > 0;y--)//刷新480行
        for(loop = y - 1;loop > 0;loop--)//
            for(x = 0;x < pic->width;x++)
                display(x,loop,pic->P_C[y][x]);
            
    return;
}



//从左至右平推输出
void Pic_Print_SpCover_Left(struct pic_info *pic)
{
    int x = 0,y = 0,loop = 0;

    for(loop = 0;loop < 800;loop++)
    {
        for(x = loop - 1;x > 0;x--)
        {
            for(y = 0 ;y < 480;y++)
                display (loop - x, y, pic->P_C[y][800 - x]);
        }
    }

    return; 
}



//进行bmp图片的特效显示
void Auto_Pic_Show(struct pic_info *pic)
{
    int ran = 0;

loop_Auto:
    ran = rand();
    switch (ran % 6)     
    {
        case 0:
            {
                if(Set_Value.set_Pic_Norm == TURE)
                {
                    Pic_Print(pic);
                    break;
                }
                else
                    goto loop_Auto;
            }
        case 1:
            {
                if(Set_Value.set_Pic_Cirle == TURE)
                {
                    Pic_Print_cirle_Auto(pic);
                    break;
                }
                else 
                    goto loop_Auto;
            }
        case 2:
            {
                if(Set_Value.set_Pic_Cover_L == TURE)
                {
                    Pic_Print_Cover_Left(pic);
                    break;
                }
                else 
                    goto loop_Auto;
            }
        case 3:
            {
                if(Set_Value.set_Pic_Many == TURE)
                {
                    Pic_Print_Many_Wind(pic);
                    break;
                }
                else 
                    goto loop_Auto;
            }
        case 4:
            {
                if(Set_Value.set_Pic_Vague_U == TURE)
                {
                    Pic_Print_Vague_Glass_down(pic);
                    break;
                }
                else 
                    goto loop_Auto;
            }
        case 5:
            {
                if(Set_Value.set_Pic_spcover == TURE)
                {
                    Pic_Print_SpCover_Left(pic);
                    break;
                }
                else
                    goto loop_Auto;
            }
    }

    return;
}







void Manu_Pic_Play(struct pic_info *pic)
{
    if(Set_Value.ev_DOWN == TURE)
    {
        Pic_Print_Vague_Glass_down(pic);
    }
    else if(Set_Value.ev_LEFT == TURE)
    {
        Pic_Print_Many_Wind (pic);
    }
    else if(Set_Value.ev_RIGTH == TURE)
    {
        Pic_Print_SpCover_Left (pic);
    }
    else if(Set_Value.ev_UP == TURE)
    {
        Pic_Print_Cover_Left (pic);
    }
    else if(Set_Value.ev_TOUCH == TURE)
    {
        Pic_Print_cirle (pic);
    }

    Set_Value.ev_DOWN = Set_Value.ev_LEFT = Set_Value.ev_RIGTH = Set_Value.ev_TOUCH = Set_Value.ev_UP = FALSE;

}











//进行单个文字与图片的合成
void Txt_Synth(int x_yuan,int y_yuan,int flag,int order,struct pic_info *pic)
{
    int x = 0,y = 0;            //所需变量的定义初始化
    int x_inc = 0,y_inc = 32;
    if(flag == CHARA)       
        x_inc = 32;     //汉字全宽
    else 
        x_inc = 16;     //其他字符半宽
    int i = x_yuan + x_inc,j = y_yuan + y_inc;  //字符边界

    
    int num = 1;
    for(y = y_yuan;y < j;y++)
    {
        for(x = x_yuan;x < i;x++)
        {
            if( Is_fu( num, flag, order) )
            {
                pic->P_C[y][x] = 0x00ff0000;
            }
                
            num ++;
            
        } 
    }
}






//判断该num中有哪些点属于需要点亮的点,并根据flag与order进行判断
int Is_fu(int num,int flag,int order)
{
	int x = num / 8;	//第几个字节
	int y = num % 8 - 1;	//第几位
	
	if(num % 8 == 0 )
	{
		x = x - 1;
		y = 7;
	}

    switch(flag)
    {
        case NUM://数字
        {
            if( txt_orig_num[order][x] & (1 << y))
        		return 1;
        	else 
        		return 0;
        }

        case LETTER://字母
        {
            if( txt_orig_letter[order - 'a'][x] & (1 << y))
        		return 1;
        	else 
        		return 0;
        }

        case CHARA://汉字
        {
            if( txt_orig_char[order][x] & (1 << y))
        		return 1;
        	else 
        		return 0;
        }

        case SPECIAL://特殊符号
        {
            if( txt_orig_spec[order][x] & (1 << y))
        		return 1;
        	else 
        		return 0;
        }
    }
}






void Txt_Ins2pic(node *pnode,struct pic_info *pic)
{
    if(Set_Value.set_Name == FALSE && Set_Value.set_PicName == FALSE)
        return;
    int len = strlen(pnode->name) + 1;

    char buf[len];
    strcpy(buf,pnode->name);        //复制完成

    int i = 0,loop = 0;
    int x_yuan = 0,y_yuan = 0;

    //printf("buf = %s\n",buf);

    if(Set_Value.set_PicName == TURE)
    {
        while(*(buf+i))
        {
            for(loop = 0;loop < 26;loop++)
            {
                if( 'a' + loop == *(buf + i) )
                {
                    Txt_Synth(x_yuan,y_yuan,LETTER,'a' + loop, pic);
                    x_yuan += 16;
                    break;
                }
                else if('0' + loop == *(buf + i))
                {
                    Txt_Synth(x_yuan,y_yuan,NUM,loop, pic);
                    x_yuan += 16;
                    break;
                }
                else if('.' == *(buf + i))
                {
                    Txt_Synth(x_yuan,y_yuan,SPECIAL,0, pic);
                    x_yuan += 16;
                    break;
                }
                else if('/' == *(buf + i))
                {
                    Txt_Synth(x_yuan,y_yuan,SPECIAL,1, pic);
                    x_yuan += 16;
                    break;
                }
            }
            i++;
        }          //名字插入完成
    }


    if(Set_Value.set_Name == TURE)
    {
        Txt_Synth(800-9*16,480-32,SPECIAL,2, pic);
        Txt_Synth(800-8*16,480-32,LETTER,'t', pic);
        Txt_Synth(800-7*16,480-32,LETTER,'i', pic);
        Txt_Synth(800-6*16,480-32,LETTER,'a', pic);
        Txt_Synth(800-5*16,480-32,LETTER,'n', pic);
        Txt_Synth(800-4*16,480-32,NUM,0, pic);
        Txt_Synth(800-3*16,480-32,NUM,2, pic);
        Txt_Synth(800-2*16,480-32,NUM,2, pic);
        Txt_Synth(800-1*16,480-32,NUM,4, pic);

    }

    //Txt_Synth(700+32,0,CHARA,0,pic);
    //Txt_Synth(700+64,0,CHARA,1,pic);

 
}


















