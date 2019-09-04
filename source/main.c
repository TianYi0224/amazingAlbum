#include "search.h"
#include "pic_handle.h"
#include "event.h"
#include "test_tian.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>





#define P_PID   10      //父进程pid存放地址


unsigned int *plcd = NULL; 		//显示器映射全局首地址
int x_tch = 0,y_tch = 0;        //点触的终点坐标
int *shmaddr = NULL;        //共享内存首地址
sem_t *sem;      //全局posix信号量
struct set_val Set_Value;       //全局设置变量

struct pic_info* pic_free = NULL;
head* Head_free = NULL;
char *path_dir_free = NULL;
int fd_free = 0;
pid_t pid;








/*
    该初始化函数进行了
    1.显示器驱动的打开以及硬件内存映射,其映射首地址为plcd
    2.申请了一块内存大小为4096字节的共享内存,其首地址为shmaddr
    3.申请了一个信号量集对上述共享内存进行了保护
    4.将用户自定义信号1与Mode_Set函数进行了绑定
*/
int  Album_Init()
{
    int fd = open("/dev/fb0",O_RDWR);
	if(fd == -1)
	{
		perror("FILE OPEN FAILED:");
		return;
	}		//显示屏驱动正常打开
	fd_free = fd;
	
	plcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(plcd == MAP_FAILED)
	{
		perror("mmap error:");
		close(fd);
		return;
	}		//显示屏硬件映射正常完成


    //申请共享内存
    //1.获取密匙
    key_t key = ftok("/Tian",0206);
    if(key == -1)
    {
        perror("KEY GET FAILED");
        return;
    }
    //2.申请/分配共享内存id
    int shmid = shmget(key,4096,IPC_CREAT|0664);//创建
    //3.映射共享内存
    shmaddr = shmat(shmid,NULL,0);
    if(shmaddr == NULL)
    {
        perror("SHMADDR GET FAILED");
        return;
    }   //共享内存映射成功

   
    //创建一个有名信号量
    sem = sem_open("/text.sem",O_CREAT,0664,1);
    if(sem == SEM_FAILED)
    {
        perror("SEM_OPEN FAILED");
        return;
    }       //信号量创建成功


    //操作共享内存,进行保护
    sem_wait(sem);
    //上锁完毕
    shmaddr[P_PID] = getpid();
    printf("P_PID in share = %d\n",shmaddr[P_PID]);
    Set_Value.set_Auto_play = shmaddr[AUTO] = TURE;    //初始化设置
    Set_Value.set_Manu_play = shmaddr[MANU] = FALSE;
    
    Set_Value.set_Pic_Norm = shmaddr[PIC_NORM] = TURE;  //默认全特效
    Set_Value.set_Pic_Cirle = shmaddr[PIC_CIRLE] = TURE;
    Set_Value.set_Pic_Cover_L = shmaddr[PIC_COVER_L] = TURE;
    Set_Value.set_Pic_Vague_U = shmaddr[PIC_VAGUE_U] = TURE;
    Set_Value.set_Pic_Many = shmaddr[PIC_MANY] = TURE;
    Set_Value.set_Pic_spcover = shmaddr[PIC_SPCOVER] = TURE;
    Set_Value.set_Name = shmaddr[NAME] = TURE;
    Set_Value.set_PicName = shmaddr[PICNAME] = FALSE;
    
    Set_Value.ev_DOWN = shmaddr[DOWN] = FALSE;  //默认自动
    Set_Value.ev_LEFT = shmaddr[LEFT] = FALSE;
    Set_Value.ev_RIGTH = shmaddr[RIGHT] = FALSE;
    Set_Value.ev_UP = shmaddr[UP] = FALSE;
    Set_Value.ev_TOUCH = shmaddr[TOUCH] = FALSE;

    Set_Value.ev_UPDATE = shmaddr[UPDATE] = FALSE;  //无更新事件
    sem_post(sem);
    //解锁


    signal(SIGUSR1,Mode_Set);   //设置事件
    signal(SIGUSR2,Update_Handle);  //手动模式下点触事件
    signal(SIGINT,Release_Resource);    //回收资源

    return fd;
}



























int main()
{
    printf("main pid = %d\n",getpid());
    int fd = Album_Init();



    pid = fork();
    if(pid <0)
    {
        perror("CHILD CREAT FAILED");
        return -1;
    }
    if(pid == 0)
    {
        int re = execl("/Tian/Hardware/touch","./touch",NULL);
        if(re == -1)
        {
            perror("EXECL CREAT FAILED");
            return;
        }
    }

    head* Head = Creat_BothWay_O_Head();		//创建链表头结点
    Head_free = Head;       //保存头结点,用于释放资源
	char *path_dir = (char *)get_current_dir_name();        //获取当前工作路径
	path_dir_free = path_dir;   //保存路径,用于释放资源
	
	Head = Find_Picture(path_dir,Head);		//寻找该工作空间下的所有bmp文件并链接
	
	White_Init();		//刷新白屏

	while(1)
	{
		Print_BothWayLinkedList(Head);
	}
    
	return 0;
}



















