#说明：该make用于一键编译当前工作目录下所有.c源文件
#并将链接好的可执行文件复制至tftp文件夹，删除中间文件



target := tian	
#最终目标名tian

cc := arm-linux-gcc
#指定编译器

csrcs := $(wildcard *.c)
#该文件夹下所有的.c文件的集合

obj := $(patsubst %.c,%.o,$(csrcs));
#所需要生成的.o中间文件的集合

path := /home/gec/tftp/
#将可执行文件复制至path

lib := -L /opt/arm_libs/lib -l jpeg  -lpthread

inc := -I../include
#头文件路径

$(target) : $(obj)
	@$(cc) -g $^ -o $@ $(inc) $(lib)
	@cp $(target) $(path)
	@rm -rf $(obj)

%.o : %.c
	@$(cc) -c $< -o $@ $(inc) $(lib)

clean:
	@rm -rf $(obj)


gdb:
	$(cc) -g $(csrcs) -o $(target) $(inc)
