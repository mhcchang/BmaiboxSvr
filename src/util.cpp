
#ifdef __LINUX_OS__
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#elif __WINDOWS_OS__
#include <Windows.h>

//
//int GetShellRes( char* sz, char* res )
//{
//    FILE   *stream;
//    FILE    *wstream;
//    char   buf[1024];
//    int streamSize;
//    memset( buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
//    stream = popen( "ls -l", "r" ); //将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream
//    streamSize = 0;
//    do
//    {
//       char* tmp = fgets(buffer,sizeof(buffer), stream);
//       streamSize += strlen(tmp);
//    } while (tmp != NULL)
//    
//    pclose( stream );
//    return streamSize;
//}

#endif