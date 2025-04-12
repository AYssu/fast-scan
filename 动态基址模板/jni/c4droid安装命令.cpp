//     注意   添加 -lmemory 就可以了 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//返回子串位置
int strpos(const char *haystack,const char *needle)  
{
	register unsigned char c, needc;
	unsigned char const *from, *end;
	int len = strlen(haystack);
	int needlen = strlen(needle);
	from = (unsigned char *)haystack;
	end = (unsigned char *)haystack + len;
	const char *findreset = needle;
	for (int i = 0; from < end; ++i) {
		c = *from++;
		needc = *needle;
		if(c == needc) {
			++needle;
			if(*needle == '\0') {
				if (len == needlen) 
					return 0;
				else
					return i - needlen+1;
			}
		}  
		else {  
			if(*needle == '\0' && needlen > 0)
				return i - needlen +1;
			needle = findreset;  
		}
	}  
	return  -1;  
} 

int main()
{
	
	if (getuid() == 0)
	{
		puts("请在非root环境下运行: 软件右上角>设置>以root身份运行>取消勾选");
		exit(1);
	}
	
	char path[64]={};
	char path_inc[256]={};
	char path_lib[256]={};
	
	const char*env=getenv("PATH");	
	int n=strpos(env,"/bintools");	
	strncpy(path,env,n);
	strcpy(path_inc,"cp ./include/*.h* ");
	strcat(path_inc,path);
	
	#if defined (__arm64__)|| defined (__aarch64__)
    printf("你的系统是arm64位: 64-bit [__arm64__ arch]\n");
    strcat(path_inc,"/gcc/aarch64-linux-android/include/");
    strcpy(path_lib,"cp ./lib/arm64-v8a/libmemory.a ");
    strcat(path_lib,path);
    strcat(path_lib,"/gcc/aarch64-linux-android/lib/");
    #else
    printf("暂不支持\n");
    return 0;
    #endif
    puts("开始安装");
    system(path_lib);
    system(path_inc);
    puts("安装成功  请到设置G++ 追加 -lmemory 库\n");
    return 0;
}