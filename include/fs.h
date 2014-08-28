#ifndef FS_H
#define FS_H

#define FT_NML	1				//普通文件
#define FT_DIR	2				//目录

struct INODE {
	unsigned int i_mode;		// 文件类型
	unsigned int i_size;		// 文件长度
	unsigned int i_block[8];
};

#define MAX_NAME_LEN 11
#define APP_SEL_STR	"0x28"
struct DIR_ENTRY {
	char de_name[MAX_NAME_LEN];
	int de_inode;
};

struct SUPER_BLOCK {
	unsigned char sb_magic;        //魔数，分区类型
	/* DPT 0x08 */
	unsigned int sb_start;			//起始扇区号
	/* DPT 0x0c */
	unsigned int sb_blocks;			//分区容量，用扇区数表示

	unsigned int sb_dmap_blks;		//块位图占用了多少块，一块为一个扇区
	unsigned int sb_imap_blks;		//inode数组位图占用了多少块;		
	unsigned int sb_inode_blks;		//inode数组占用了多少块
};

#define ABS_BOOT_BLK(sb)		((sb).sb_start)
#define ABS_SUPER_BLK(sb)		((ABS_BOOT_BLK(sb))+1)
#define ABS_DMAP_BLK(sb)		((ABS_SUPER_BLK(sb))+1)
#define ABS_IMAP_BLK(sb)		((ABS_DMAP_BLK(sb))+(sb).sb_dmap_blks)
#define ABS_INODE_BLK(sb)		((ABS_IMAP_BLK(sb))+(sb).sb_imap_blks)
#define ABS_DATA_BLK(sb)		((ABS_INODE_BLK(sb))+INODE_BLKS)

#define INODE_BIT_BLKS		1	/* 512*8 = 4096 inodes */
#define INODES_PER_BLK		(512/sizeof(struct INODE))
#define INODE_BLKS			((INODE_BIT_BLKS*512*8+INODES_PER_BLK-1)/(INODES_PER_BLK))

extern struct INODE iroot;

struct SUPER_BLOCK SB_FIRST;

void stat(int dir_num);
void verify_fs(void);
void verify_dir(void);
unsigned int alloc_blk(struct SUPER_BLOCK *sb);
void free_blk(struct SUPER_BLOCK *sb, unsigned int n);
void install_color(void);

#define PATH_LEN 30
struct PS
{
	char path[PATH_LEN];
	int clen;
}PS1;

int rep_enter;					//按enter键是否重复执行上条命令
int current_inode;
int last_inode;
void touch(int inode_num,char *file_name);
int fhave(int inode_num,char *filename);
int fcat(char *filename);
void write(char *filename,char *content);
int scan_files(int dir_num);
int rm_file(int inode_num,char *file_name);
void mkdir(int inode_num,char *dir_name);
void cd(char *dir_name);
void mv(char *filename,char *dirname);
int find_empty(int dir_num);
int dhave(int inode_num,char *filename);
int cp(char *filename,char *dirname);
void init_PS1();
void str_append(struct PS *PS1,char *dir);
void tail_sub(struct PS *PS1);
void putsPS1();

#endif