#include "search.h"
#include "pic_handle.h"
#include "event.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <jpeglib.h>
#include <semaphore.h>





extern struct set_val Set_Value;
extern sem_t *sem;
extern int *shmaddr;



//该源文件用于实现所有bmp/jpeg图片的查找/搜索/判断                   以及链表的实现
//1.Is_Bmp
//2.Is_Jpeg
//3.Find_Picture
//4.Creat_BothWay_O_Head
//5.Print_BothWayLinkedList
//6.Destroy_BothWay_Head
//7.Add_To_BothWay_Head





//通过文件路径判断一个文件是否为bmp文件
//标准bmp文件
int Is_Bmp(char *path)
{
	int fd = open(path,O_RDONLY);
	if(fd == -1)
	{
		perror("FILE OPEN FAILED:");
		return -1;
	}		//成功打开文件
	
	int flag1 = 1;
	int flag2 = 0;
	
	char *buf = malloc( sizeof(char)*3 );
	buf[2] = 0;
	int seek = lseek(fd,0,SEEK_SET);
	if(seek == 0)		//文件偏移量 偏移成功
	{
		int r = read(fd,buf,2);		//读取文件中的第一个/第二个字节
		if(r == -1)		//读取失败
		{
			perror("FILE READ FAILED:");
			close(fd);
			return -1;
		}
		
		flag1 = strcmp(buf,"BM") && strcmp(buf,"BA") && strcmp(buf,"CI") && strcmp(buf,"CP") && strcmp(buf,"IC") && strcmp(buf,"PT");
	}
	
	int *num = malloc( sizeof(int) );
	seek = lseek(fd,0x0e,SEEK_SET);
	if(seek == 0x0e)		//偏移成功
	{
		int r = read(fd,num,sizeof(int));
		if (r == -1)
		{
			perror("FILE READ FAILED:");
			close(fd);
			return -1;
		}
		
		if(*num == 40)
			flag2 = 1;
	}		//完成bmp文件的检测
	
	free(num);		//释放变量
	free(buf);
	close(fd);
	
	if(flag1 == 0 && flag2 == 1)		//符合bmp文件标准,为标准bmp文件
		return 1;
		
	return 0;
}



//以后缀判断一个文件是否为jpeg文件,标准jpeg文件返回1,否则返回0
int Is_Jpeg(char *path)
{
    if( !strcmp(".jpeg",path + strlen(path) - 5) )
        return 1;
    else 
        return 0;
}





//该函数用于寻找整个文件夹下的所有bmp以及jpeg图片文件,并使用链表将其路径链接
//参数:根文件夹路径,链表头结点                 返回值:链表头结点
head* Find_Picture(char *path_dir,head *Head)
{
	DIR *dirp = opendir(path_dir);
	if(dirp == NULL)		//打开文件夹失败
	{
		perror("DIR OPEN FAILED:");
		return;
	}
	
	struct dirent *dirent = NULL;
	struct stat st; 
	
	while(dirent = readdir(dirp))		//逐一循环读取该文件夹下的文件
	{	
		char path_name[1000] = {0};
		strcpy(path_name,path_dir);
		
		strcat(path_name,"/");		//完成文件路径的连接
		strcat(path_name,dirent -> d_name);		//完成文件路径的连接
		
		int stat_re = stat(path_name,&st);
		if(stat_re == 0)		//获取文件属性成功
		{
			if( S_ISREG(st.st_mode) )		//判断普通文件
			{
				if( Is_Bmp(path_name) == 1 )
				{		//确认为bmp文件
				    if( !strcmp(dirent -> d_name,"set.bmp") )
                        continue;
					Head = Add_To_BothWay_Head(Head,dirent -> d_name,path_name);
					continue;
				}
                
                if( Is_Jpeg (path_name) == 1 )
                {
                    Head = Add_To_BothWay_Head(Head,dirent -> d_name,path_name);
                    continue;
                }

                
			}
			else if(  S_ISDIR(st.st_mode) )		//判断文件夹
			{
				if( !strcmp(dirent -> d_name,".") || !strcmp(dirent -> d_name,"..") )		//舍弃.以及..
					continue;
				
				Head = Find_Picture(path_name,Head);
				//printf("DIR:%s   \n",dirent -> d_name);		//打印文件夹名
			}
		}
	}
	closedir(dirp);
	
	return Head;
}




//创建一个带头结点的双向链表头结点,注意该头节点在销毁函数中进行了释放,无需手动释放
head* Creat_BothWay_O_Head()
{
	head* hd = malloc(sizeof(head));
	hd -> num = 0;
	hd -> first = hd -> last = NULL;
	strcpy(hd -> name,"The path of bmp");
	
	//printf("please input the name:\n");
	//scanf("%s",hd -> name);
	
	return hd;
}



//简单按照链表顺序升序显示图片
void Print_BothWayLinkedList(head *hd)
{
    struct pic_info* pic = NULL;
	node *p = hd -> first;
	
	//printf("正序输出为:\n");
	while(p)
	{
        if( !strcmp(".bmp",p->name + strlen(p->name) - 4) )     //判断图片并获取颜色
            pic = Bmp_Get_Color(p->path,TURE);
        else if( !strcmp(".jpeg",p->name + strlen(p->name) - 5) )
            pic = Jpeg_Get_Color(p->path,TURE);
        //完成图片像素数据的采集

        Txt_Ins2pic (p, pic);


replay:
        if(Set_Value.set_Auto_play == TURE)
        {       //自动随机特效播放
            Auto_Pic_Show (pic);
        }
        else if(Set_Value.set_Manu_play == TURE)
        {       //手动播放为真
            while(1)    //等待点触事件发生
            {
                if(Set_Value.ev_UPDATE == TURE)
                {
                    Set_Value.ev_UPDATE = FALSE; //响应此次更新

                    //在此次手动播放等待中,发生了播放模式的切换
                    if(Set_Value.set_Auto_play == TURE)
                    {
                        sem_wait(sem);  //上锁,表示已完成此次按键更新
                        shmaddr[UPDATE] = FALSE;
                        sem_post(sem);  //解锁
                        goto replay;
                    }
                        

                    //否则为普通点触事件
                    Manu_Pic_Play(pic);//待完成
                    printf("PLAY LOADING\n");
                    sem_wait(sem);  //上锁,表示已完成此次按键更新
                    shmaddr[UPDATE] = FALSE;
                    sem_post(sem);  //解锁
                    break;
                }
            }
        }

        sleep(2);
		p = p -> next;
        free(pic);
	}
	printf("\n");
}





//销毁一个带头结点的双向链表,连带表头
void Destroy_BothWay_Head(head *hd)
{	
	node *p = hd -> first;
	
	while(hd -> first)
	{
		p = hd -> first;
		hd -> first = hd -> first -> next;
		
		p -> prev = p -> next = NULL;
		hd -> num--;
        free(p->name);      //数据释放
        free(p->path);
		free(p);
	}
	
	hd -> first = hd -> last = NULL;
	hd -> num = 0;
	free(hd);
}



//该函数用于向链表尾部添加指定节点
//hd:头节点 name:文件名 path:文件路径
head* Add_To_BothWay_Head(head *hd,Elemtype name,Elemtype path)
{
	node *pnew = malloc(sizeof(*pnew));		//创建该节点
	pnew -> next = pnew -> prev = NULL;		//指针域
	
	pnew -> name = malloc( strlen(name) +1 );		//数据域
	pnew -> path = malloc( strlen(path) +1 );
	strcpy(pnew -> name,name);
	strcpy(pnew -> path,path);		//完成节点的创建
	
	
	if(hd -> first == NULL)		//插入节点
	{
		hd -> first = hd -> last = pnew;
	}
	else
	{
		hd -> last -> next = pnew;
		pnew -> prev = hd -> last;
		hd -> last = pnew;
	}
	
	hd -> num++;
	
	return hd;
}














