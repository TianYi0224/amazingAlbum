#ifndef _SEARCH_H_
#define _SEARCH_H_





//类型声明
typedef char* Elemtype;

typedef struct node_both_way
{
	Elemtype name;
	Elemtype path;
	struct node_both_way *next;
	struct node_both_way *prev;
}node;

typedef struct head
{
	char name[32];
	int num;
	node* first;
	node* last;
}head;


//函数原型声明,其函数实现在具体源文件中
int Is_Bmp(char *path);

int Is_Jpeg(char *path);

head* Find_Picture(char *path_dir,head *Head);

head* Creat_BothWay_O_Head();

void Print_BothWayLinkedList(head *hd);

void Destroy_BothWay_Head(head *hd);

head* Add_To_BothWay_Head(head *hd,Elemtype name,Elemtype path);













#endif
