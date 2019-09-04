#ifndef _EVENT_H_
#define _EVENT_H_


#define UP 		1   //0001  点触事件宏取缔
#define DOWN 	2   //0010
#define LEFT 	4   //0100
#define RIGHT 	8   //1000
#define TOUCH	0   //0000

#define SETING  11  //设置模式宏
#define SET_FINISH  12  //设置结束宏
#define UPDATE  13  //更新事件
#define AUTO    14  //自动模式
#define MANU    15  //手动模式
#define PIC_NORM    16
#define PIC_CIRLE   17
#define PIC_COVER_L 18
#define PIC_VAGUE_U 19
#define PIC_MANY    20  //百叶窗
#define PIC_SPCOVER 21  //平推
#define NAME    22
#define PICNAME 23

#define X_TCH   24
#define Y_TCH   25



struct set_val
{
    int ev_UPDATE;
    int ev_TOUCH;
    int ev_UP;
    int ev_DOWN;
    int ev_RIGTH;
    int ev_LEFT;
    int set_Auto_play;
    int set_Manu_play;
    int set_Pic_Norm;
    int set_Pic_Cirle;
    int set_Pic_Cover_L;
    int set_Pic_Vague_U;
    int set_Pic_Many;
    int set_Pic_spcover;
    int set_Name;
    int set_PicName;
};



void Mode_Set(int sign);

void Update_Handle(int sign);

void Release_Resource(int sign);





#endif