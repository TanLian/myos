直接make生成final.img后，就可以在虚拟机中运行
文件系统：
	可使用touch命令，如touch a
	可使用cp命令，如cp a b
	可使用mv命令，如mv b c
	可使用write命令，向一个文件中写内容，如write "hello world" >> a
	可使用cat命令，读一个文件的内容，如cat a
	可使用rm命令，如rm a
	
	欢迎大家实现更多的命令。
	
不足：
	fork系统调用未实现
	没有分页