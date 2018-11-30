NAME=time_modifier
obj-m += $(NAME).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o time_modifier_command_line time_modifier_command_line.c -std=c11
	gcc -o time_modifier_gui time_modifier_gui.c -Wall `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -export-dynamic -rdynamic

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f  time_modifier_command_line
	rm -f  time_modifier_gui
