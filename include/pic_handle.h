#ifndef _PIC_HANDLE_H_
#define _PIC_HANDLE_H_

#define TURE    1       //用于表示是否进行居中显示
#define FALSE   0

#define NUM     1       //用于表示字符显示中调用那种数组
#define LETTER  2
#define CHARA   3
#define SPECIAL 4



//变量类型声明
struct pic_info
{
    int width;  //像素宽度,对应x
    int length; //像素长度,对应y
    int depth;  //色深
    int start_x;    //显示起始点x
    int start_y;    //显示起始点y
    int P_C[480][800];//像素显示中,行为第一要素,列为第二要素
};


//函数原型声明

struct pic_info* Bmp_Get_Color(char *path,int flag);

struct pic_info* Jpeg_Get_Color(char *path,int flag);

void Start_Check(struct pic_info* pic);

void display(int x,int y,int color);

void center_change(struct pic_info* pic);

void White_Init();

void Pic_Print(struct pic_info *pic);

void Pic_Print_cirle(struct pic_info *pic);

void Pic_Print_cirle_Auto(struct pic_info *pic);

void Pic_Print_Cover_Left(struct pic_info *pic);

void Pic_Print_Many_Wind(struct pic_info *pic);

void Pic_Print_Vague_Glass_down(struct pic_info *pic);

void Pic_Print_Vague_Glass_up(struct pic_info *pic);

void Pic_Print_SpCover_Left(struct pic_info *pic);

void Auto_Pic_Show(struct pic_info *pic);

void Txt_Synth(int x_yuan,int y_yuan,int flag,int order,struct pic_info *pic);

int Is_fu(int num,int flag,int order);

void Txt_Ins2pic(node *, struct pic_info *);


#endif