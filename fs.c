#include <fs.h>
#include <kprintf.h>
#include <kernel.h>
#include <hd.h>
#include <libcc.h>
#include <x86.h>
#include <mm.h>
#include <task.h>

int current_inode = 0;
int last_inode = -1;

void setb(void *s, unsigned int i)
{
	unsigned char *v = s;
	v += i>>3;
	*v |= 1<<(7-(i%8));
}

void clrb(void *s, unsigned int i)
{
	unsigned char *v = s;
	v += i>>3;
	*v &= ~(1<<(7-(i%8)));
}

int testb(void *s, unsigned int i)
{
	unsigned char *v = s;
	v += i>>3;
	return (*v&(1<<(7-(i%8)))) != 0;
}

void verify_fs(void)
{
	unsigned int *q = (unsigned int *)(HD0_ADDR);
	unsigned char sect[512] = {0};
	struct SUPER_BLOCK sb;
	unsigned int i = 0, j = 0, m = 0, n = 0;

	/* get the fs sb */
	sb.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, &sb);
	/* the first partition must be FST_FS type */
	if (sb.sb_magic != FST_FS) {
		//kprintf(KPL_DUMP, "Partition 1 does not have a valid file system\n");
		//kprintf(KPL_DUMP, "Creating file system\t\t\t\t\t\t\t  ");
		sb.sb_magic = FST_FS;
		sb.sb_start = q[0];
		sb.sb_blocks = q[1];
		sb.sb_dmap_blks = (sb.sb_blocks+0xfff)>>12;
		sb.sb_imap_blks = INODE_BIT_BLKS;
		sb.sb_inode_blks = INODE_BLKS;
		hd_rw(ABS_SUPER_BLK(sb), HD_WRITE, 1, &sb);

		/* initial data bitmap */
		n = ABS_DMAP_BLK(sb);
		j = sb.sb_dmap_blks+sb.sb_imap_blks+sb.sb_inode_blks+2;
		memset(sect, 0xff, sizeof sect/sizeof sect[0]);
		for (i=j/(512*8); i>0; --i) {
			hd_rw(n++, HD_WRITE, 1, sect);
			m += 4096;
		}
		m += 4096;
		for (i=j%(512*8); i<512*8; ++i) {
			clrb(sect, i);
			--m;
		}
		hd_rw(n++, HD_WRITE, 1, sect);

		memset(sect, 0, sizeof sect/sizeof sect[0]);
		for (i=sb.sb_imap_blks-(n-ABS_DMAP_BLK(sb)); i>0; --i)
			hd_rw(n++, HD_WRITE, 1, sect);

		/* initail inode bitmap */
		for (i=sb.sb_imap_blks; i>0; --i)
			hd_rw(ABS_IMAP_BLK(sb)+i-1, HD_WRITE, 1, sect);

		/* initial inode blocks */
		for (i=sb.sb_inode_blks; i>0; --i)
			hd_rw(ABS_INODE_BLK(sb)+i-1, HD_WRITE, 1, sect);
		//kprintf(KPL_DUMP, "[DONE]");
	}
	q += 2;

	/*kprintf(KPL_DUMP, "0: Type: FST_FS ");
	kprintf(KPL_DUMP, "start at: %d ", sb.sb_start);
	kprintf(KPL_DUMP, "blocks: %d ", sb.sb_blocks);
	kprintf(KPL_DUMP, "dmap: %d ", sb.sb_dmap_blks);
	kprintf(KPL_DUMP, "imap: %d ", sb.sb_imap_blks);
	kprintf(KPL_DUMP, "inodes: %d\n", sb.sb_inode_blks);*/

	/* initialize swap partition */
	sb.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, &sb);
	if (sb.sb_magic != FST_SW) {
		//kprintf(KPL_DUMP, "\nPartition 2 does not have a valid file system\n");
		//kprintf(KPL_DUMP, "Creating file system\t\t\t\t\t\t\t  ");
		sb.sb_magic = FST_SW;
		sb.sb_start = q[0];
		sb.sb_blocks = q[1];
		sb.sb_dmap_blks = (sb.sb_blocks)>>15;	/* 1 bits == 4K page */
		hd_rw(ABS_SUPER_BLK(sb), HD_WRITE, 1, &sb);
		//kprintf(KPL_DUMP, "[DONE]");	
	}
	/* initial data bitmap */
	n = ABS_DMAP_BLK(sb);
	j = sb.sb_dmap_blks+2;
	memset(sect, 0xff, sizeof sect/sizeof sect[0]);
	for (i=j/(512*8); i>0; --i) {
		hd_rw(n++, HD_WRITE, 1, sect);
		m += 4096;
	}
	m += 4096;
	for (i=j%(512*8); i<512*8; ++i) {
		clrb(sect, i);
		--m;
	}
	hd_rw(n++, HD_WRITE, 1, sect);

	/*kprintf(KPL_DUMP, "1: Type: FST_SW ");
	kprintf(KPL_DUMP, "start at: %d ", sb.sb_start);
	kprintf(KPL_DUMP, "blocks: %d ", sb.sb_blocks);
	kprintf(KPL_DUMP, "dmap: %d, presents %d 4k-page\n",
			sb.sb_dmap_blks, sb.sb_blocks>>3);*/
}

unsigned int alloc_blk(struct SUPER_BLOCK *sb)
{
	//找到第一个找到的未用块，返回它的相对于整个盘的LBA地址
	unsigned int i = 0, j = 0, n = 0, m = 0;
	unsigned char sect[512] = {0};

	n = ABS_DMAP_BLK(*sb);
	for (; i<sb->sb_dmap_blks; ++i) {
		hd_rw(n, HD_READ, 1, sect);
		for (j=0; j<512*8; ++j) {
			if (testb(sect, j)) {
				++m;
			} else {			/* gotcha */
				setb(sect, j);
				if (m >= sb->sb_blocks)
					return 0;
				else {
					hd_rw(n, HD_WRITE, 1, sect);
					return ABS_BOOT_BLK(*sb) + m;
				}
			}
		}
		++n;
	}
	return 0;
}

void free_blk(struct SUPER_BLOCK *sb, unsigned int n)
{
	unsigned char sect[512] = {0};
	unsigned int t = (n-ABS_BOOT_BLK(*sb))/(512*8)+ABS_DMAP_BLK(*sb);
	hd_rw(t, HD_READ, 1, sect);
	clrb(sect, (n-ABS_BOOT_BLK(*sb))%(512*8));
	hd_rw(t, HD_WRITE, 1, sect);
}

int alloc_inode(struct SUPER_BLOCK *sb)
{
	//在Imap里面遍历，找到第一个非0的项，返回
	unsigned char sect[512] = {0};
	int i = 0;
	hd_rw(ABS_IMAP_BLK(*sb), HD_READ, 1, sect);
	for (; i<512; ++i) {
		if (! testb(sect, i)) {
			setb(sect, i);
			hd_rw(ABS_IMAP_BLK(*sb), HD_WRITE, 1, sect);
			break;
		}
	}
	return (i==512)?-1:i;
}

void free_inode(struct SUPER_BLOCK *sb, int n)
{
	unsigned char sect[512] = {0};
	hd_rw(ABS_IMAP_BLK(*sb), HD_READ, 1, sect);
	clrb(sect, n);
	hd_rw(ABS_IMAP_BLK(*sb), HD_WRITE, 1, sect);
}

struct INODE *iget(struct SUPER_BLOCK *sb, struct INODE *inode, int n)
{
	unsigned char sect[512] = {0};
	int i = n/INODES_PER_BLK;
	int j = n%INODES_PER_BLK;

	hd_rw(ABS_INODE_BLK(*sb)+i, HD_READ, 1, sect);
	memcpy(inode, sect+j*sizeof(struct INODE), sizeof(struct INODE));
	return inode;
}

void iput(struct SUPER_BLOCK *sb, struct INODE *inode, int n)
{
	unsigned char sect[512] = {0};
	int i = n/INODES_PER_BLK;
	int j = n%INODES_PER_BLK;
	hd_rw(ABS_INODE_BLK(*sb)+i, HD_READ, 1, sect);
	memcpy(sect+j*sizeof(struct INODE), inode, sizeof(struct INODE));
	hd_rw(ABS_INODE_BLK(*sb)+i, HD_WRITE, 1, sect);
}

struct INODE iroot = {FT_DIR, 2*sizeof(struct DIR_ENTRY), {0,}};

void stat(int dir_num)
{
	struct DIR_ENTRY *de;
	struct INODE finode,inode;
	unsigned int i = 0;
	char sect[512] = {0};
	char imap[512] = {0};
	char *type[2] = {"FILE","DIR"};

		
	//从硬盘中加载位图到sect数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, imap);

	iget(&SB_FIRST,&inode,dir_num);
	switch (inode.i_mode) {
	case FT_DIR:
		hd_rw(inode.i_block[0], HD_READ, 1, sect);
		kprintf(KPL_DUMP, "\nNAME\tSIZE\tMODE\tINODE\n");
		de = (struct DIR_ENTRY *)sect;
		//打印.和..两项			
		for (i=0; i<2; ++i) 
			kprintf(KPL_DUMP, "%s\t%d\t%s\t%x\n", de[i].de_name,inode.i_size, type[1],de[i].de_inode);
		int count = 2;
		int i = 2;
		while(count < scan_files(dir_num))
		{
			if(testb(imap,de[i].de_inode) == 0)
			{
				i++;
				continue;
			}
			iget(&SB_FIRST,&finode,de[i].de_inode);
			kprintf(KPL_DUMP, "%s\t%d\t%s\t%x\n", de[i].de_name, finode.i_size,type[finode.i_mode-1],de[i].de_inode);
			count++;
			i++;
		}		
		break;
	default:
		break;
	}
}

void check_root(void) 
{
	unsigned char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;

	//填充超级块SB_FIRST
	SB_FIRST.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(SB_FIRST), HD_READ, 1, sect);
	memcpy(&SB_FIRST, sect, sizeof(struct SUPER_BLOCK));
	
	//从硬盘中加载位图到sect数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, sect);
	
	
	if (! testb(sect, 0)) {
		//kprintf(KPL_DUMP, "/ has not been created, creating....\t\t\t\t\t  ");
		if (alloc_inode(&SB_FIRST) != 0) {
			//kprintf(KPL_PANIC, "\n/ must be inode 0!!!\n");
			hlt();
		}
		iroot.i_block[0] = alloc_blk(&SB_FIRST);
		iput(&SB_FIRST, &iroot, 0);
		
		hd_rw(iroot.i_block[0], HD_READ, 1, sect);
		de = (struct DIR_ENTRY *)sect;
		strcpy(de->de_name, ".");
		de->de_inode = 0;
		++de;
		strcpy(de->de_name, "..");
		de->de_inode = -1;
		hd_rw(iroot.i_block[0], HD_WRITE, 1, sect);
		//kprintf(KPL_DUMP, "[DONE]");
		//if (iroot.i_size == 2*sizeof(struct DIR_ENTRY))
			//install_color();
	}
	iget(&SB_FIRST, &iroot, 0);
	iroot.i_size = scan_files(0) * sizeof(struct DIR_ENTRY);
	iput(&SB_FIRST, &iroot, 0);
	
	printk("/");
	init_PS1();
	
	//以下代码可删除，用来测验的
	iget(&SB_FIRST, &iroot, 0);
	hd_rw(iroot.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;

	if ((strcmp(de[0].de_name, ".")) || (de[0].de_inode) ||
		(strcmp(de[1].de_name, "..")) || (de[1].de_inode) != -1) {
		//kprintf(KPL_PANIC, "File system is corrupted!!!\n");
		kprintf(KPL_DUMP, "File system is corrupted!!!\n");
		hlt();
	}

}

void
verify_dir(void) {
	/*unsigned char sect[512] = {0};
	unsigned int *q = (unsigned int *)(HD0_ADDR);
	struct INODE inode;
	struct SUPER_BLOCK SB_FIRST;

	SB_FIRST.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(SB_FIRST), HD_READ, 1, sect);*/
	check_root();
	//memcpy(&SB_FIRST, sect, sizeof(struct SUPER_BLOCK));
	//stat(iget(&SB_FIRST, &inode, 0));
}

void touch(int inode_num,char *file_name)
{
		//printk("touch");
	struct INODE f_inode,tmp_inode;
	int empty;
	char *c;
	char sect[512] = {0};
	struct DIR_ENTRY *dir = NULL;
	int num_inode = -1;	
	unsigned int blk = 0;	
	int i = 0;
	
		//printk("11111");
	//如果该文件早已经存在，那还新建个毛线啊
	if(fhave(inode_num,file_name) != -1)
	{
		printk("\n%s already exist!",file_name);
		return;
	}
		//printk("22222");
		//printk("33333");
	//在给定目录下添加一个记录（文件名和inode号），并写入到硬盘
	iget(&SB_FIRST,&tmp_inode,inode_num);
	hd_rw(tmp_inode.i_block[0], HD_READ, 1, sect);
	empty = find_empty(inode_num);
	//printk("empty:%d\n",empty);
		//printk("44444");
	//分配一个inode号
	num_inode = alloc_inode(&SB_FIRST);
		//printk("num_inode:%d\n",num_inode);
	assert(num_inode > 0);
		//printk("before blk:%d\n",blk);
	//分配一个块号（扇区号）
	blk = alloc_blk(&SB_FIRST);
		//printk("blk:%d\n",blk);
	assert(blk != 0);
		//printk("55555");
	//填充struct inode结构体，并把它写入硬盘
	f_inode.i_block[0] = blk;
	f_inode.i_mode = FT_NML;
	f_inode.i_size = 0;
	iput(&SB_FIRST, &f_inode, num_inode);		
		//printk("66666");

	dir = &((struct DIR_ENTRY *)sect)[empty];
	c = file_name;
	while((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z'))
	{
		dir->de_name[i++] = *c;
		c++;
	}
		//printk("111111");
	dir->de_name[i] = '\0';
	dir->de_inode = num_inode;
	hd_rw(tmp_inode.i_block[0], HD_WRITE, 1, sect);

	//重新设置给定inode的size

	//tmp_inode.i_size += 0/*sizeof(struct DIR_ENTRY)*/;
	iput(&SB_FIRST, &tmp_inode, inode_num);
}

int fhave(int inode_num,char *filename)
{
	//根据iroot的i_block[0]找到对应的名字，返回对应的inode号
	char *c;
	struct INODE inode;
	int count,i;
	char *f = filename;
	char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;	
	char imap[512] = {0};
		
	//从硬盘中加载位图到sect数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, imap);
	
	iget(&SB_FIRST,&inode,inode_num);
	hd_rw(inode.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;
	
	count = 2;
	i = 2;
	while(count < scan_files(inode_num))
	{
		if(testb(imap,de[i].de_inode) == 0)
		{
			i++;
			continue;
		}
				
		c = de[i].de_name;
		while(*c == *f && *c != '\0')
		{
			c++;
			f++;
		}

		if(*c == '\0'/* && *f == '\0'*/)
		{
			return de[i].de_inode;
		}
			
		count++;
		i++;
	}
	
	return -1;
}

int dhave(int inode_num,char *filename)
{
	//根据给定目录的i_block[0]找到对应的名字，返回对应的inode号
	char *c;
	struct INODE inode,tmp_inode;
	int count,i = 0;
	char *f = filename;
	char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;	
	char imap[512] = {0};
		char lastdir[2];
		while(i < 2)
		{
			lastdir[i] = *f;
			f++;
			i++;
		}
		f = filename;
		i = 0;
	
		
	//从硬盘中加载位图到sect数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, imap);
	
	iget(&SB_FIRST,&inode,inode_num);
	hd_rw(inode.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;
	
	if(strncmp(lastdir,"..",2) == 0 && inode_num == 0)
	{
		printk("\nYou can't cd .. when you are in /");
		return 0;
	}
	else if(strncmp(lastdir,"..",2) == 0)
	{
		tail_sub(&PS1);
		return de[1].de_inode;
	}
	else if(strncmp(lastdir,".",1) == 0)
	{
		return de[0].de_inode;
	}
	
	count = 0;
	i = 0;
	while(count < scan_files(inode_num))
	{
		if(testb(imap,de[i].de_inode) == 0)
		{
			i++;
			continue;
		}
				
		c = de[i].de_name;
		while(*c == *f && *c != '\0')
		{
			c++;
			f++;
		}

		if(*c == '\0'/* && *f == '\0'*/)
		{
			iget(&SB_FIRST,&tmp_inode,de[i].de_inode);
			if(tmp_inode.i_mode == FT_DIR)
			{
				//str_append(&PS1,de[i].de_name); 
				return de[i].de_inode;
			}
		}
			
		count++;
		i++;
	}
	
	return -1;
}

int fcat(char *filename)
{
	char *c;
	int len = 0;
	struct INODE inode;
	char sect[512] = {0};
	
	int inode_num = fhave(current_inode,filename);
	
	//如果没有该文件，那还查看个毛啊
	if(inode_num == -1)
	{
		printk("don't have!");
		return -1;
	}
	
	//把指定inode值（这里是inode_num）的inode内容从磁盘读入内存（填充inode结构体）
	iget(&SB_FIRST, &inode, inode_num);

	hd_rw(inode.i_block[0], HD_READ, 1, sect);
	
	c = sect;
	while(*c != '\0' && len < 512)
	{
		printk("%c",*c);
		c++;
		len++;
	}
	
	return 1;
}

void write(char *filename,char *content)
{
	struct INODE finode,cdir_inode;
	char sect[512] = {0};
	
	//根据文件名获得inode号
	int inode_num = fhave(current_inode,filename);
	//没有该文件，写毛啊？
	if(inode_num == -1)
	{
		printk("\ndon't have!");
		return;
	}
	
	//根据inode号获得对应的struct inode结构体
	iget(&SB_FIRST, &finode, inode_num);
	
	if(finode.i_mode == FT_DIR)
	{
		printk("\nYou can't write sth to DIR");
		return;
	}
	
	//重新设置finode的大小
	int count = 0;
	char *c = content;
	while(*c != '\0')
	{
		count++;
		c++;
	}
	finode.i_size = count * sizeof(char);
	iput(&SB_FIRST, &finode, inode_num);
	
	iget(&SB_FIRST,&cdir_inode,current_inode);
	cdir_inode.i_size += finode.i_size;
	iput(&SB_FIRST,&cdir_inode,current_inode);
	
	//将content写入到对应的扇区
	hd_rw(finode.i_block[0], HD_WRITE, 1, content);
}

//给定一个目录inode号，返回该目录下的文件数
int scan_files(int dir_num)
{
	struct INODE dir_inode;
	unsigned char sect[512] = {0};
	unsigned char imap[512] = {0};
	struct DIR_ENTRY *de = NULL;
	int i = 0;
	int count = 2;
	
	//从硬盘中加载位图到imap数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, imap);
			
	iget(&SB_FIRST,&dir_inode,dir_num);
	hd_rw(dir_inode.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;
	i = 2;
	while(i < 512)
	{
		if(sizeof(de[i].de_name) <= 0 || de[i].de_inode <= 0)
			break;
		if(testb(imap,de[i].de_inode))
		{			
			count++;
		}

		i++;
	}

	return count;
}

int rm_file(int num_inode,char *file_name)
{
	/*删除文件主要做三件事：
		1.清除imap中相应的位（即inode号）
		2.清除dmap中相应的位
		3.清除iroot的i_block[0]中相应扇区的数据
	*/
	struct INODE finode,cdir_inode;
	char sect[512] = {0};
	
	//根据文件名获得inode号
	int inode_num = fhave(num_inode,file_name);
	//没有该文件，删除毛啊？
	if(inode_num == -1)
	{
		printk("\ndon't have!");
		return;
	}
	
	iget(&SB_FIRST,&finode,inode_num);
	
	free_inode(&SB_FIRST,inode_num);
	free_blk(&SB_FIRST,finode.i_block[0]);
	
	//重新设置该文件所在的size
	iget(&SB_FIRST, &cdir_inode, num_inode);
	cdir_inode.i_size -= finode.i_size;
	iput(&SB_FIRST, &cdir_inode, num_inode);
}

/*
新建目录：
	类似于Linux的当前工作路径，我们就来一个当前工作INODE号，初始值为iroot的地址
	每调用一次cd命令，我们就切换该值
*/

/*在当前工作路径上新建一个目录:
	在当前目录下添加一个记录（目录名和inode号），并写入到硬盘
	建.和..两个文件
	
*/
void mkdir(int inode_num,char *dir_name)
{
	struct INODE d_inode;
	int num_inode = -1;
	int i = 0;
	struct DIR_ENTRY *dir = NULL;
	unsigned char sect[512] = {0};
	unsigned int blk = 0;

	iget(&SB_FIRST, &d_inode, inode_num);

	int empty = find_empty(inode_num);
	//printk("empty:%d\n",empty);
	
	//分配一个inode号
	num_inode = alloc_inode(&SB_FIRST);
	assert(num_inode > 0);
	
	//分配一个块号（扇区号）
	blk = alloc_blk(&SB_FIRST);
	assert(blk != 0);
	
	//填充struct inode结构体，并把它写入硬盘
	d_inode.i_block[0] = blk;
	d_inode.i_mode = FT_DIR;
	d_inode.i_size = 2*sizeof(struct DIR_ENTRY);
	iput(&SB_FIRST, &d_inode, num_inode);		
	
	//在当前目录下添加一个记录（目录名和inode号），并写入到硬盘
	iget(&SB_FIRST, &d_inode, inode_num);
	hd_rw(d_inode.i_block[0], HD_READ, 1, sect);
	
	dir = &((struct DIR_ENTRY *)sect)[empty];
	char *c = dir_name;
	while((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z'))
	{
		dir->de_name[i++] = *c;
		c++;
	}
	dir->de_name[i] = '\0';
	dir->de_inode = num_inode;
	hd_rw(d_inode.i_block[0],HD_WRITE, 1, sect);

	//重新设置current_inode的size
	d_inode.i_size += sizeof(struct DIR_ENTRY);
	iput(&SB_FIRST, &d_inode, inode_num);
	
	//新建.和..两个文件（为了节约inode号，.和新建的这个目录的inode号是一样的）
	hd_rw(blk, HD_READ, 1, sect);
	dir = (struct DIR_ENTRY *)sect;
	strcpy(dir->de_name, ".");
	dir->de_inode = num_inode;
	++dir;
	strcpy(dir->de_name, "..");
	dir->de_inode = inode_num;
	hd_rw(blk, HD_WRITE, 1, sect);
}

/*cd到一个目录：
	主要是改变current_inode和last_inode的值（目前只支持相对目录切换）
*/
void cd(char *dir_name)
{
	int num_inode = dhave(current_inode,dir_name);
	if(num_inode == -1)
	{
		printk("\ndon't have!");
		return;
	}
	char *c = dir_name;
	char name[12];
	int i = 0;
	while((*c >= 'a' && *c <= 'z')||(*c >= 'A' && *c <= 'Z'))
	{
		name[i] = *c;
		i++;
		c++;
	}
	name[i] = '\0';
	c = dir_name;
	if(*c != '.')
		str_append(&PS1,name); 
	
	last_inode = current_inode;
	current_inode = num_inode;
}

/*cp的实现：如cp file tanlian/
*/
int cp(char *filename,char *dirname)
{
	struct INODE finode,tmpnode;
	char sect[512] = {0};
	char dirsect[512] = {0};
	
	//获得目录INODE号
	int dnode_num = dhave(current_inode,dirname);
	if(dnode_num == -1)
	{
		printk("\nNo such DIR");
		return -1;
	}
	
	//获得文件INODE号
	int fnode_num = fhave(current_inode,filename);
	if(fnode_num == -1)
	{
		printk("/nNo such file");
		return -1;
	}
	//目标目录已存在该文件，则不需要拷贝
	if(fhave(dnode_num,filename) != -1)
	{
		printk("\nFILE has exsited in Dest DIR");
		return -1;
	}
	
	//获得struct INODE结构体
	iget(&SB_FIRST,&finode,fnode_num);
	
	//从硬盘读出数据到内存（sect）中
	hd_rw(finode.i_block[0], HD_READ, 1, sect);
	
	touch(dnode_num,filename);
	
	//获得文件INODE号
	int tmp_num = fhave(dnode_num,filename);
	//printk("tmp_num:%d\n",tmp_num);
	//获得struct INODE结构体
	iget(&SB_FIRST,&tmpnode,tmp_num);
	//从硬盘读出数据到内存（sect）中
	hd_rw(tmpnode.i_block[0], HD_WRITE, 1, sect);
	tmpnode.i_mode = finode.i_mode;
	tmpnode.i_size = finode.i_size;
	//写回硬盘
	iput(&SB_FIRST,&tmpnode,tmp_num);
}

/*mv的实现，如mv aaa tanlian/
*/
void mv(char *filename,char *dirname)
{
	cp(filename,dirname);
	rm_file(current_inode,filename);
}

/*
找到一个空项
*/
int find_empty(int dir_num)
{
	struct INODE dir_inode;
	unsigned char sect[512] = {0};
	unsigned char imap[512] = {0};
	struct DIR_ENTRY *de = NULL;
	int i = 0;
	
	iget(&SB_FIRST,&dir_inode,dir_num);
	
	//从硬盘中加载位图到imap数组
	hd_rw(ABS_IMAP_BLK(SB_FIRST), HD_READ, 1, imap);
				
	hd_rw(dir_inode.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;
	i = 2;
	while(i < scan_files(dir_num))
	{
		if(! testb(imap,de[i].de_inode))
		{			
			return i;
		}

		i++;
	}

	return i;
}

void init_PS1()
{
	PS1.clen = 0;
}

void str_append(struct PS *PS1,char *dir)
{
	char *c = dir;
	PS1->path[PS1->clen] = '/';
	while(*c != '\0' && ++PS1->clen < PATH_LEN)
	{
		PS1->path[PS1->clen] = *c;
		c++;
	}
	PS1->path[++PS1->clen] = '/';
}

void tail_sub(struct PS *PS1)
{
	char *c;
	PS1->path[PS1->clen] = '\0';
	PS1->clen--;
	c = PS1->path + PS1->clen;
	
	while(*c != '/')
	{
		PS1->path[PS1->clen] = '\0';
		PS1->clen--;
		c--;
	}
}

void putsPS1()
{
	char *c = PS1.path;
	while(*c != '\0')
	{
		printk("%c",*c);
		c++;
	}
}
/*
void cmd_app(int inode_num)
{
	struct INODE inode;
	struct SUPER_BLOCK SB_FIRST;
	char sect[512] = {0};

	//填充超级块sb
	SB_FIRST.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(SB_FIRST), HD_READ, 1, sect);
	memcpy(&SB_FIRST, sect, sizeof(struct SUPER_BLOCK));

	//把指定inode值的inode内容从磁盘读入内存，填充inode结构体
	iget(&SB_FIRST, &inode, inode_num);

	//char p[512] = {0xf,};
	//char q[4*1024] = {0xf,};
	char *p = mm_alloc(memman,inode.i_size);
	char *q = mm_alloc(memman,4*1024);

	//将硬盘中的内容读到内存中
	hd_rw(inode.i_block[0], HD_READ, 1, p);

	setgdt(5,inode.i_size,(int)p,0x40fa);
	setgdt(6,4*1024,(int)q,0x40f2);

	__asm__ __volatile__("ljmp	$" APP_SEL_STR ",	$0\n\t");
		printk("after app\n");
	//mm_free(memman,(int)p,inode.i_size);
	//mm_free(memman,(int)q,4*1024);
	return;
}

void install_color(void)
{
	struct SUPER_BLOCK SB_FIRST;
	char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;
	int inode = -1;
	int empty = -1;
	struct INODE clnode;
	unsigned int blk = 0;
	unsigned char color[] = {0x57,0x56,0x53,0xb8,0x00,0x00,0x00,0x00,0xb9,0x00,0x00,0x00,0x00,0xbb,0x54,0x00,0x00,0x00,0xba,0x02,0x00,0x00,0x00,0x89,0xc6,0x89,0xcf,0xcd,0x80,0xeb,0xfe,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x7a,0x52,0x00,0x01,0x7c,0x08,0x01,0x1b,0x0c,0x04,0x04,0x88,0x01,0x00,0x00,0x1c,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0xc0,0xff,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x41,0x0e,0x08,0x87,0x02,0x41,0x0e,0x0c,0x86,0x03,0x41,0x0e,0x10,0x83,0x04,};

	SB_FIRST.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(SB_FIRST), HD_READ, 1, sect);
	memcpy(&SB_FIRST, sect, sizeof(struct SUPER_BLOCK));

	empty = find_empty(0);
	
	inode = alloc_inode(&SB_FIRST);
	assert(inode > 0);
	blk = alloc_blk(&SB_FIRST);
	assert(blk != 0);
	clnode.i_block[0] = blk;
	hd_rw(blk, HD_WRITE, 1, color);
	clnode.i_mode = FT_NML;
	clnode.i_size = sizeof color;
	iput(&SB_FIRST, &clnode, inode);		//把clnode写入硬盘

	hd_rw(iroot.i_block[0], HD_READ, 1, sect);
	de = &((struct DIR_ENTRY *)sect)[empty];
	strcpy(de->de_name, "color");
	de->de_inode = inode;
	hd_rw(iroot.i_block[0], HD_WRITE, 1, sect);

	iget(&SB_FIRST, &iroot, 0);
	iroot.i_size += sizeof(struct DIR_ENTRY);
	iput(&SB_FIRST, &iroot, 0);
	
	printk("install color\n");
}
*/
