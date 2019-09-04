#include "event.h"
#include "search.h"
#include "pic_handle.h"
#include <stdio.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>




extern int x_tch,y_tch; //声明外部全局变量,用于记录最终触点坐标
extern unsigned int *plcd;  //声明外部内存映射全局变量
extern int *shmaddr;        //声明外部共享内存首地址
extern sem_t *sem;      //声明用于保护上述共享内存的信号量集
extern struct set_val Set_Value;    //声明模式设置全局变量

extern struct pic_info* pic_free;
extern head* Head_free;
extern char *path_dir_free;
extern int fd_free;
extern pid_t pid;


//该源文件用于实现触摸屏的检测
//




//该函数用于处理来自触摸屏的设置事件
void Mode_Set(int sign)
{
    shmaddr[SETING] = TURE;     //标识进入设置模式


    int x = 0,y = 0;
    int buf[480][800] = {0};//暂存当前颜色数组
    for(y = 0;y < 480 ;y++)
        for(x = 0;x < 800; x++)
            buf[y][x] = *(plcd+800*y+x);



    struct pic_info *pic =  Bmp_Get_Color("/Tian/set.bmp",TURE);

    struct pic_info pic_org = *pic;//结构体属于一等公民,可以使用等号进行赋值操作

    struct set_val temp;
    temp.set_Auto_play = shmaddr[AUTO];
    temp.set_Manu_play = shmaddr[MANU];
    temp.set_Pic_Many = shmaddr[PIC_MANY];
    temp.set_Pic_spcover = shmaddr[PIC_SPCOVER];
    temp.set_Pic_Cirle = shmaddr[PIC_CIRLE];
    temp.set_Pic_Cover_L = shmaddr[PIC_COVER_L];
    temp.set_Pic_Norm = shmaddr[PIC_NORM];
    temp.set_Pic_Vague_U = shmaddr[PIC_VAGUE_U];
    temp.set_Name = shmaddr[NAME];
    temp.set_PicName = shmaddr[PICNAME];
    
    

    //显示设置
    if(temp.set_Auto_play == TURE)
        Txt_Synth(135,40,CHARA,2,pic);
    if(temp.set_Manu_play == TURE)
        Txt_Synth(135,165,CHARA,2,pic);
    if(temp.set_Pic_Norm == TURE)
        Txt_Synth (415, 30, CHARA, 2, pic);
    if(temp.set_Pic_Cirle == TURE)
        Txt_Synth (415, 120, CHARA, 2, pic);
    if(temp.set_Pic_Cover_L == TURE)
        Txt_Synth (415, 195, CHARA, 2, pic);
    if(temp.set_Pic_Vague_U == TURE)
        Txt_Synth (415, 260, CHARA, 2, pic);
    if(temp.set_Pic_Many == TURE)
        Txt_Synth(415,360,CHARA,2,pic);
    if(temp.set_Pic_spcover == TURE)
        Txt_Synth(415,430,CHARA,2,pic);
    if(temp.set_Name == TURE)
        Txt_Synth(115,400,CHARA,2,pic);
    if(temp.set_PicName = TURE)
        Txt_Synth(115,265,CHARA,2,pic);
    Pic_Print(pic);
    

    while(1)
    {
        sem_wait(sem);
        // 上锁,进行数据的更新,防止在更新设置时,点触进程抢占,导致无法预测的错误
        if(shmaddr[SET_FINISH] == TURE) //检测到退出标志位被设置
        {
            sem_post(sem);  //解锁,防止带锁退出
            break;
        }
        temp.set_Auto_play = shmaddr[AUTO];
        temp.set_Manu_play = shmaddr[MANU];
        temp.set_Pic_Norm = shmaddr[PIC_NORM];
        temp.set_Pic_Cirle = shmaddr[PIC_CIRLE];
        temp.set_Pic_Cover_L = shmaddr[PIC_COVER_L];
        temp.set_Pic_Vague_U = shmaddr[PIC_VAGUE_U];
        temp.set_Pic_Many = shmaddr[PIC_MANY];
        temp.set_Pic_spcover = shmaddr[PIC_SPCOVER];
        temp.set_Name = shmaddr[NAME];
        temp.set_PicName = shmaddr[PICNAME];
        sem_post(sem);  //读取结束,解锁,防止带锁退出

        *pic = pic_org;  //pic设置图像回滚

        if(temp.set_Auto_play == TURE)  //显示设置更改
            Txt_Synth(130,48,CHARA,2,pic);
        if(temp.set_Manu_play == TURE)
            Txt_Synth(135,165,CHARA,2,pic);
        if(temp.set_Pic_Norm == TURE)
            Txt_Synth (405, 30, CHARA, 2, pic);
        if(temp.set_Pic_Cirle == TURE)
            Txt_Synth (405, 111, CHARA, 2, pic);
        if(temp.set_Pic_Cover_L == TURE)
            Txt_Synth (405, 190, CHARA, 2, pic);
        if(temp.set_Pic_Vague_U == TURE)
            Txt_Synth (405, 265, CHARA, 2, pic);
        if(temp.set_Pic_Many == TURE)
            Txt_Synth(405,353,CHARA,2,pic);
        if(temp.set_Pic_spcover == TURE)
            Txt_Synth(405,430,CHARA,2,pic);
        if(temp.set_Name == TURE)
            Txt_Synth(130,386,CHARA,2,pic);
        if(temp.set_PicName == TURE)
            Txt_Synth(130,265,CHARA,2,pic);
        Pic_Print(pic);
    }

    Set_Value = temp;       //同步至全局变量
    Set_Value.ev_UPDATE = TURE; //标记产生更新事件

    for(y = 0;y < 480 ;y++)     //图像回滚
        for(x = 0;x < 800; x++)
            *(plcd+800*y+x) = buf[y][x];


    sem_wait(sem);  //上锁,设置退出标志位
    shmaddr[SETING] = FALSE;    //标识退出设置模式
    shmaddr[SET_FINISH] = FALSE;
    sem_post(sem);  //解锁,完成退出设置



}






void Update_Handle(int sign)
{
    sem_wait(sem);  //上锁,更新事件至本地全局变量
    Set_Value.ev_UPDATE = shmaddr[UPDATE];
    Set_Value.ev_DOWN = shmaddr[DOWN];
    Set_Value.ev_LEFT = shmaddr[LEFT];
    Set_Value.ev_RIGTH = shmaddr[RIGHT];
    Set_Value.ev_UP = shmaddr[UP];
    Set_Value.ev_TOUCH = shmaddr[TOUCH];    //完成事件的更新
    x_tch = shmaddr[X_TCH];
    y_tch = shmaddr[Y_TCH];

    shmaddr[DOWN] = shmaddr[UP] = shmaddr[LEFT] = shmaddr[RIGHT] = shmaddr[TOUCH] = FALSE;
    sem_post(sem);  //解锁,完成更新,但共享内存中UPDATE标志并未清空

}








void Release_Resource(int sign)
{
    //接收到结束信息,进行资源的释放
    printf("realeasing source\n");
    free(pic_free);     //释放图片信息
    free(path_dir_free);    //释放路径名
    Destroy_BothWay_Head(Head_free);    //释放链表
    munmap(plcd,800*480*4);     
    close(fd_free);     //关闭显示屏驱动文件

    int re = kill(pid,SIGINT);//发送信号进入资源回收模式
    if(re == -1)
        perror("KILL SIGNAL SEND FAILED");

    printf("killing son process\n");
    int status;
    wait(&status);
    printf("killed\n");

    
    shmdt(shmaddr);     //关闭共享内存
    sem_unlink("/text.sem");    //删除信号量
    

    printf("exit sucsess\n");
    exit(3);









}

































