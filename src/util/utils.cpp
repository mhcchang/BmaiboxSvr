#include <vector>
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sstream>

#ifdef __WINDOWS_OS__
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <errno.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/if_ether.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <sys/time.h>
//#include <sys/reboot.h>
#include <unistd.h>
#endif

//#define SIOCGIFNAME		0x8910		get iface name		
//#define SIOCSIFLINK		0x8911		 set iface channel	
//#define SIOCGIFCONF		0x8912		get iface list		
//#define SIOCGIFFLAGS		0x8913		get flags			
//#define SIOCSIFFLAGS		0x8914		 set flags			
//#define SIOCGIFADDR		0x8915		get PA address		
//#define SIOCSIFADDR		0x8916		 set PA address		
//#define SIOCGIFDSTADDR	0x8917		get remote PA address	
//#define SIOCSIFDSTADDR	0x8918		 set remote PA address	
//#define SIOCGIFBRDADDR	0x8919		get broadcast PA address
//#define SIOCSIFBRDADDR	0x891a		 set broadcast PA address
//#define SIOCGIFNETMASK	0x891b		get network PA mask		
//#define SIOCSIFNETMASK	0x891c		 set network PA mask		

#define LINUX_REBOOT_CMD_RESTART        0x01234567
#define LINUX_REBOOT_CMD_HALT           0xCDEF0123
#define LINUX_REBOOT_CMD_CAD_ON         0x89ABCDEF
#define LINUX_REBOOT_CMD_CAD_OFF        0x00000000
#define LINUX_REBOOT_CMD_POWER_OFF      0x4321FEDC
#define LINUX_REBOOT_CMD_RESTART2       0xA1B2C3D4
#define LINUX_REBOOT_CMD_SW_SUSPEND     0xD000FCE2
#define LINUX_REBOOT_CMD_KEXEC          0x45584543

const std::vector<std::string> c_log_level_str =
{
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL",
	"OFF"
};

//控制信息类型
const std::vector<std::string> MsgTypeChar = {
	"unknown",
	"/config/getBoxId",
	"/config/setBoxId",
	"/config/getSysTime",
	"/config/setSysTime",
	"/config/setSysNtp", //5
	"/config/reboot",
	"/config/getBasicInfo",
	"/config/softUpdate",
	"/config/algoUpdate",
	"/config/setNetwork", //10
	"/config/getNetwork",
	"/config/setMqtt",
	"/config/getMqtt",
	"/config/setPassword",
	"/config/checkPassword", //15
	"/config/getDeviceStatus",
	"/config/addCamera",
	"/config/delCamera",
	"/config/getCameras",
	"/config/addRoi", //20
	"/config/delRoi",
	"/config/getRoi",
	"/config/getSnapshot",
	"/config/setArmRule",
	"/config/getArmRule", //25
	"/config/setAlgoRule",
	"/config/getAlgoRule",
	"/alarms/count",
	"/alarms/query",
	"/notification/alarm", //30
	"/notification/statusChange/channel",
	"/notification/statusChange/aiBox"
};

#ifdef __WINDOWS_OS__
void changeWorkingDirectory()
{
	TCHAR path[MAX_PATH];
	if (GetModuleFileNameA(NULL, path, MAX_PATH))
	{
		int len = lstrlenA(path);
		while (len > 0 && path[len] != '\\')
		{
			path[len] = '\0';
			--len;
		}

		SetCurrentDirectoryA(path);
	}
}

std::string UTF8ToGBK(const char* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);

	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;

	return strTemp;
}

/*
GBK 转 UTF-8
*/
std::string GBKToUTF8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	std::string strTemp = str;

	if (wstr) delete[] wstr;
	if (str) delete[] str;

	return strTemp;
}
#else
#endif

time_t StringToTimet(std::string str)
{
	char * cha = (char*)str.data(); // 将string转换成char*。
	tm tm_; // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900; // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1; // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day; // 日。
	tm_.tm_hour = hour; // 时。
	tm_.tm_min = minute; // 分。
	tm_.tm_sec = second; // 秒。
	tm_.tm_isdst = 0; // 非夏令时。
	time_t t_ = mktime(&tm_); // 将tm结构体转换成time_t格式。
	return t_; // 返回值。
}

tm StringToTm(std::string str)
{
	char * cha = (char*)str.data(); // 将string转换成char*。
	tm tm_; // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900; // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1; // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day; // 日。
	tm_.tm_hour = hour; // 时。
	tm_.tm_min = minute; // 分。
	tm_.tm_sec = second; // 秒。
	tm_.tm_isdst = 0; // 非夏令时。

	return tm_;
}

void BinToHexStr(unsigned char * Bin, int nlen, unsigned char * Hex)
{
	int i;
	int j;

	for (i = 0; i < nlen; i++)
	{
		j = (Bin[i] >> 4) & 0xf;

		if (j <= 9)
		{
			Hex[i * 2] = (j + '0');
		}
		else
		{
			Hex[i * 2] = (j + 'a' - 10);
		}

		j = Bin[i] & 0xf;

		if (j <= 9)
		{
			Hex[i * 2 + 1] = (j + '0');
		}
		else
		{
			Hex[i * 2 + 1] = (j + 'a' - 10);
		}
	};

	Hex[nlen * 2] = '\0';
}

bool HexStrToBin(char * str, unsigned char * Bin)
{
	int i;
	int ilen = strlen(str) / 2;
	if (strlen(str) % 2 == 0)
	{
		return false;
	}

	for (i = 0; i < ilen; i++)
	{
		if (str[i * 2] >= '0' && str[i * 2] <= '9')
		{
			Bin[i] = (str[i * 2] - '0') << 4;
		}
		else if (str[i * 2] >= 'a' && str[i * 2] <= 'z')
		{
			Bin[i] = (str[i * 2] - 'a') << 4;
		}
		else
		{
			return false;
		}

		if (str[i * 2 + 1] >= '0' && str[i * 2 + 1] <= '9')
		{
			Bin[i] |= (str[i * 2 + 1] - '0');
		}
		else if (str[i * 2 + 1] >= 'a' && str[i * 2 + 1] <= 'z')
		{
			Bin[i] |= str[i * 2 + 1] - 'a';
		}
		else
		{
			return false;
		}
	}

	return true;
}

int GetShellRes(char* szCmd, std::stringstream &sstream)
{
	sstream.clear();
    FILE * stream;
    char buf[1024];
    int streamSize;
    memset( buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
    stream = popen(szCmd, "r" ); //将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream
    streamSize = 0;
	char* tmp = NULL;
	while ((tmp = fgets(buf, sizeof(buf), stream)) != NULL)
	{
		sstream << tmp;
		//printf("GetShellRes %s \n", buf);
		streamSize += strlen(tmp);
	}
    pclose( stream );
    return streamSize;
}

/************************************************
	char *pt="2006-4-20 20:30:30";
	SetSystemTime(pt);
**************************************************/
int SetSystemTime(char *dt)
{
	struct tm rtctm;
	struct tm _tm;
	struct timeval tv;
	time_t timep;
	sscanf(dt, "%d-%d-%d %d:%d:%d", &rtctm.tm_year,
		&rtctm.tm_mon, &rtctm.tm_mday, &rtctm.tm_hour,
		&rtctm.tm_min, &rtctm.tm_sec);
	_tm.tm_sec = rtctm.tm_sec;
	_tm.tm_min = rtctm.tm_min;
	_tm.tm_hour = rtctm.tm_hour;
	_tm.tm_mday = rtctm.tm_mday;
	_tm.tm_mon = rtctm.tm_mon - 1;
	_tm.tm_year = rtctm.tm_year - 1900;

	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	if (settimeofday(&tv, (struct timezone *) 0) < 0)
	{
		printf("Set system datatime error!/n");
		return -1;
	}
	return 0;
}

#ifndef LINUX_REBOOT_CMD_RESTART
#define	LINUX_REBOOT_CMD_RESTART	0x01234567
#endif
#ifndef LINUX_REBOOT_CMD_HALT
#define	LINUX_REBOOT_CMD_HALT	0xCDEF0123
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
#define	LINUX_REBOOT_CMD_CAD_ON	0x89ABCDEF
#endif
#ifndef LINUX_REBOOT_CMD_CAD_OFF
#define LINUX_REBOOT_CMD_CAD_OFF	0x00000000
#endif
#ifndef LINUX_REBOOT_CMD_POWER_OFF
#define LINUX_REBOOT_CMD_POWER_OFF	0x4321FEDC
#endif
#ifndef LINUX_REBOOT_CMD_RESTART2
#define LINUX_REBOOT_CMD_RESTART2	0xA1B2C3D4
#endif
#ifndef LINUX_REBOOT_CMD_SW_SUSPEND
#define LINUX_REBOOT_CMD_SW_SUSPEND 0xD000FCE2
#endif
//关机

bool powerOffProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_POWER_OFF) == -1 ? false : true;
}
//注销
bool logOffProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_RESTART) == -1 ? false : true;
}

//重启
bool reBootProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_RESTART) == -1 ? false : true;
}

void WriteReportEvent(char* szName, char* szFunction)
{
	return;
}
#define MAC_SIZE 18
#define IP_SIZE 16
int get_ip_by_domain(const char *domain, char *ip)
{
	char **pptr;
	struct hostent *hptr;

	hptr = gethostbyname(domain);
	if (NULL == hptr)
	{
		printf("gethostbyname error for host:%s/n", domain);
		return -1;
	}

	for (pptr = hptr->h_addr_list; *pptr != NULL; pptr++)
	{
		if (NULL != inet_ntop(hptr->h_addrtype, *pptr, ip, IP_SIZE))
		{
			return 0; // 只获取第一个 ip
		}
	}

	return -1;
}
//原文链接：https ://blog.csdn.net/hhl0529/article/details/108353146

int get_local_mac(const char *eth_inf, char *mac)
{
	struct ifreq ifr;
	int sd;

	bzero(&ifr, sizeof(struct ifreq));
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("get %s mac address socket creat error\n", eth_inf);
		return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

	if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("get %s mac address error\n", eth_inf);
		close(sd);
		return -1;
	}

	snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
		(unsigned char)ifr.ifr_hwaddr.sa_data[0],
		(unsigned char)ifr.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr.ifr_hwaddr.sa_data[2],
		(unsigned char)ifr.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	close(sd);

	return 0;
}

int get_local_ip(const char *eth_inf, char *ip)
{
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
		printf("socket error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ioctl error: %s\n", strerror(errno));
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

	close(sd);
	return 0;
}

int getmacaddr(uint8_t *s, size_t n, const char *ifname)
{
	struct ifreq ifreq;		/* interface request */
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sfd)
		return -1;
	strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));
	/* Get hardware address	*/
	if (-1 == ioctl(sfd, SIOCGIFHWADDR, &ifreq)) {
		close(sfd);
		return -1;
	}
	memcpy(s, ifreq.ifr_hwaddr.sa_data, n);
	return 0;
}
//https ://blog.csdn.net/weixin_45206746/article/details/116759349
/*
int getifInfo()
{
	struct ifaddrs *ifa, *node;
	struct sockaddr_in *sin;
	uint8_t mac[IFHWADDRLEN];
	if (-1 == getifaddrs(&ifa))
		return 0;

	int i = 0;
	for (node = ifa; node != NULL; node = node->ifa_next) {
		if (AF_INET != node->ifa_addr->sa_family)
			continue;
		if (-1 == getmacaddr(mac, sizeof(mac), node->ifa_name))
			return -1;
		// print out IF messages 
		putchar('\n');
		printf("ifname: %s\n", node->ifa_name);
		sin = (struct sockaddr_in*)node->ifa_addr;
		printf("ifv4: %s\n", inet_ntoa(sin->sin_addr));
		sin = (struct sockaddr_in*)node->ifa_broadaddr;
		printf("boradcast: %s\n", inet_ntoa(sin->sin_addr));
		sin = (struct sockaddr_in*)node->ifa_netmask;
		printf("subnet mask: %s\n", inet_ntoa(sin->sin_addr));
		printf("MAC: %02x.%02x.%02x.%02x.%02x.%02x\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

		i++;
	}
	freeifaddrs(ifa);

	return i;
}

//原文链接：https ://blog.csdn.net/weixin_45206746/article/details/116759349
int getifInfo(char* ifr_name, char* addr, char* mask, char* mac, char* states)
{
	struct ifreq ifr;
	int sock, j, k, flags;
	int mtu;
	char *p;// , addr[32], mask[32], mac[32], states[100];

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == sock) 
	{
		perror("socket() ");
		return -1;
	}

	strncpy(ifr.ifr_name, ifr_name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	if (-1 == ioctl(sock, SIOCGIFADDR, &ifr)) 
	{
		perror("ioctl(SIOCGIFADDR) ");
		return -1;
	}
	p = inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
	strncpy(addr, p, 32 - 1);
	addr[sizeof(addr) - 1] = '\0';

	if (-1 == ioctl(sock, SIOCGIFNETMASK, &ifr)) 
	{
		perror("ioctl(SIOCGIFNETMASK) ");
		return -1;
	}

	p = inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr);
	strncpy(mask, p, 32 - 1);
	mask[sizeof(mask) - 1] = '\0';

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) 
	{
		perror("ioctl(SIOCGIFFLAGS) ");
		return -1;
	}
	flags = ifr.ifr_flags;
	if (flags & IFF_UP)
		strcpy(states, "UP,");
	else
		strcpy(states, "DOWN,");
	if (flags & IFF_BROADCAST)
		strcat(states, "BROADCAST,");
	if (flags & IFF_LOOPBACK)
		strcat(states, "LOOPBACK,");
	if (flags & IFF_RUNNING)
		strcat(states, "RUNNING,");
	if (flags & IFF_NOARP)
		strcat(states, "NOARP,");
	if (flags & IFF_MULTICAST)
		strcat(states, "MULTICAST,");
	if (states[strlen(states) - 1] == ',')
		states[strlen(states) - 1] = ' ';
	else
		states[strlen(states) - 1] = 0;

	if (-1 == ioctl(sock, SIOCGIFHWADDR, &ifr)) 
	{
		perror("ioctl(SIOCGIFHWADDR) ");
		return -1;
	}
	for (j = 0, k = 0; j < 6; j++) 
	{
		k += snprintf(mac + k, sizeof(mac) - k - 1, j ? ":%02X" : "%02X", (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
	}
	mac[sizeof(mac) - 1] = '\0';

	if (ioctl(sock, SIOCGIFMTU, &ifr) < 0)
		mtu = 0;
	else
		mtu = ifr.ifr_mtu;

	printf("ifname: %s\n", ifr.ifr_name);
	printf("address: %s\n", addr);
	printf("netmask: %s\n", mask);
	printf("macaddr: %s\n", mac);
	printf("mtu: %d \n", mtu);
	printf("state: %s\n", states);
	printf("flags: %d\n", flags);

	close(sock);
	return 0;
}

int SetIfAddr(char *ifname, char *Ipaddr, char *mask, char *gateway)
{
	int fd;
	int rc = 0;
	struct ifreq ifr;
	struct sockaddr_in *sin;
	struct rtentry rt;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		perror("socket   error");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	sin->sin_family = AF_INET;

	//ipaddr
	if (inet_aton(Ipaddr, &(sin->sin_addr)) < 0)
	{
		perror("inet_aton   error");
		return -2;
	}

	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
	{
		perror("ioctl   SIOCSIFADDR   error");
		return -3;
	}

	//netmask
	if (inet_aton(mask, &(sin->sin_addr)) < 0)
	{
		perror("inet_pton   error");
		return -4;
	}

	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
	{
		perror("ioctl");
		return -5;
	}

	//gateway
	memset(&rt, 0, sizeof(struct rtentry));
	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	if (inet_aton(gateway, &sin->sin_addr) < 0)
	{
		printf("inet_aton error\n");
		return -6;
	}

	memcpy(&rt.rt_gateway, sin, sizeof(struct sockaddr_in));
	((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
	rt.rt_flags = RTF_GATEWAY;

	if (ioctl(fd, SIOCADDRT, &rt) < 0)
	{
		perror("ioctl(SIOCADDRT) error in set_default_route\n");
		close(fd);
		return -1;
	}

	close(fd);
	return rc;
}


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <net/route.h>
#include <unistd.h>
#include <stdio.h>

int set_gateway(unsigned long gw)
{
	int skfd;
	struct rtentry rt;
	int err;

	skfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
		return -1;

	//Delete existing defalt gateway 
	memset(&rt, 0, sizeof(rt));

	rt.rt_dst.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;

	rt.rt_genmask.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;

	rt.rt_flags = RTF_UP;

	err = ioctl(skfd, SIOCDELRT, &rt);

	if ((err == 0 || errno == ESRCH) && gw) {
		// Set default gateway
		memset(&rt, 0, sizeof(rt));

		rt.rt_dst.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;

		rt.rt_gateway.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;

		rt.rt_genmask.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_genmask) - > sin_addr.s_addr = 0;

		rt.rt_flags = RTF_UP | RTF_GATEWAY;

		err = ioctl(skfd, SIOCADDRT, &rt);
	}

	close(skfd);

	return err;
}

int get_gateway(unsigned long *p, char* gateway)
{
	FILE *fp;
	char buf[256];    
	char iface[16];
	unsigned long dest_addr, gate_addr;
	*p = INADDR_NONE;
	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
		return -1;

	fgets(buf, sizeof(buf), fp);
	while (fgets(buf, sizeof(buf), fp)) 
	{
		if (sscanf(buf, "%s\t%lX\t%lX", iface, &dest_addr, &gate_addr) != 3 ||
			dest_addr != 0)
			continue;
		*p = gate_addr;
		struct sockaddr_in sin;
		sin.sin_addr = *(in_addr*)p;

		char* tmp = inet_ntoa(sin.sin_addr);
		if (!tmp)
		{
			printf("inet_ntoa error\n");
			return -2;
		}
		strncpy(gateway, tmp, strlen(tmp) );
		
		break;
	}

	fclose(fp);
	return 0;
}

int SetDns(char *dns)
{
	FILE *fp;
	char buf[256] = { 0 }; // 128 is enough for linux    
	char iface[16];
	unsigned long dest_addr, gate_addr;
	fp = fopen("/etc/systemd/resolved.conf", "rw");
	if (fp == NULL)
		return -1;

	using namespace std;
	bool bl = false;
	while (fgets(buf, sizeof(buf), fp))
	{
		if (strncmp(buf, "DNS=", 4) == 0)
		{
			strcpy(buf + 4, dns);

			fwrite(buf, strlen(buf) + 1, 1, fp);
			bl = true;
			break;
		}
	}

	if (!bl)
	{
		sprintf(buf, "DNS=%s", dns);
		fwrite(buf, strlen(buf) + 1, 1, fp);
	}
	fclose(fp);

	system("echo linaro | sudo systemctl restart systemd-resolved");
	system("echo linaro | sudo systemctl enable systemd-resolved");
	system("echo linaro | sudo mv /etc/resolv.conf /etc/resolv.conf.bak");
	system("echo linaro | sudo ln -s /run/systemd/resolve/resolv.conf /etc/");

	return 0;
}

int GetDns(char *dns)
{
	FILE *fp;
	char buf[256]; // 128 is enough for linux    
	char iface[16];
	unsigned long dest_addr, gate_addr;
	fp = fopen("/etc/systemd/resolved.conf", "r");
	if (fp == NULL)
		return -1;

	using namespace std;

	while (fgets(buf, sizeof(buf), fp)) 
	{
		if (strncmp(buf, "DNS=", 4) == 0)
		{
			char* ip = strstr(buf + 4, " ");
			if (ip)
			{
				strncpy(dns, buf + 4, ip - buf + 4);
			}
			else
			{
				strncpy(dns, buf + 4, strlen(buf) - 4);
			}

			struct sockaddr_in sin;
			memset(&sin, 0, sizeof(struct sockaddr_in));
			sin.sin_family = AF_INET;
			sin.sin_port = 0;
			if (inet_aton(buf + 4, &sin.sin_addr) < 0)
			{
				printf("inet_aton error\n");
				return -2;
			}

			break;
		}
	}

	fclose(fp);
	return 0;
}
*/
typedef struct cpu_occupy_          //定义一个cpu occupy的结构体
{
	char name[20];                  //定义一个char类型的数组名name有20个元素
	unsigned int user;              //定义一个无符号的int类型的user
	unsigned int nice;              //定义一个无符号的int类型的nice
	unsigned int system;            //定义一个无符号的int类型的system
	unsigned int idle;              //定义一个无符号的int类型的idle
	unsigned int iowait;
	unsigned int irq;
	unsigned int softirq;
}cpu_occupy_t;

double cal_cpuoccupy(cpu_occupy_t *o, cpu_occupy_t *n)
{
	double od, nd;
	double id, sd;
	double cpu_use;

	od = (double)(o->user + o->nice + o->system + o->idle + o->softirq + o->iowait + o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
	nd = (double)(n->user + n->nice + n->system + n->idle + n->softirq + n->iowait + n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od

	id = (double)(o->idle);    //用户第一次和第二次的时间之差再赋给id
	sd = (double)(n->idle);    //系统第一次和第二次的时间之差再赋给sd
	if ((nd - od) != 0)
	{
		auto t1_busy = od - id;
		auto t2_busy = nd - sd;
		auto busy_delta = t2_busy - t1_busy;
		auto all_delta = nd - od;
		if (t2_busy <= t1_busy)
			return 0.0;
		cpu_use = busy_delta / all_delta * 100.0;
		//cpu_use =1- (sd-id)/(nd-od)*100.00;
	}
	else
		cpu_use = 0;
	return cpu_use;
}

void get_cpuoccupy(cpu_occupy_t *cpust)
{
	FILE *fd;
	int n;
	char buff[256];
	cpu_occupy_t *cpu_occupy;
	cpu_occupy = cpust;

	fd = fopen("/proc/stat", "r");
	if (fd == NULL)
	{
		perror("fopen:");
		exit(0);
	}
	fgets(buff, sizeof(buff), fd);

	sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->iowait, &cpu_occupy->irq, &cpu_occupy->softirq);

	fclose(fd);
}

double get_sysCpuUsage()
{
	cpu_occupy_t cpu_stat1;
	cpu_occupy_t cpu_stat2;
	double cpu;
	get_cpuoccupy((cpu_occupy_t *)&cpu_stat1);
	usleep(1000);
	//第二次获取cpu使用情况
	get_cpuoccupy((cpu_occupy_t *)&cpu_stat2);

	//计算cpu使用率
	cpu = cal_cpuoccupy((cpu_occupy_t *)&cpu_stat1, (cpu_occupy_t *)&cpu_stat2);

	return cpu;
}

#define BUFFER_LENGTH 8192

int IsDir(std::string path)
{
	if (path.empty())
	{
		return 0;
	}
	struct stat st;
	if (0 != stat(path.c_str(), &st))
	{
		return 0;
	}
	if (S_ISDIR(st.st_mode))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CopyFile(std::string sourcePath, std::string destPath)
{
	int len = 0;
	FILE *pIn = NULL;
	FILE *pOut = NULL;
	char buff[BUFFER_LENGTH] = { 0 };

	if ((pIn = fopen(sourcePath.c_str(), "r")) == NULL)
	{
		printf("Open File %s Failed...\n", sourcePath.c_str());
		return;
	}
	if ((pOut = fopen(destPath.c_str(), "w")) == NULL)
	{
		printf("Create Dest File Failed...\n");
		return;
	}

	while ((len = fread(buff, sizeof(char), sizeof(buff), pIn)) > 0)
	{
		fwrite(buff, sizeof(char), len, pOut);
	}
	fclose(pOut);
	fclose(pIn);
}

void CopyFolder(std::string sourcePath, std::string destPath)
{
	struct dirent* filename = NULL;
	if (0 == opendir(destPath.c_str()))
	{
		if (mkdir(destPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO))
		{
			printf("Create Dir Failed...\n");
		}
		else
		{
			printf("Creaat Dir %s Successed...\n", destPath.c_str());
		}
	}
	std::string path = sourcePath;
	if (sourcePath.back() != '/')
	{
		sourcePath += "/";
	}
	if (destPath.back() != '/')
	{
		destPath += "/";
	}

	DIR* dp = opendir(path.c_str());
	while (filename = readdir(dp))
	{
		std::string fileSourceFath = sourcePath;

		std::string fileDestPath = destPath;

		fileSourceFath += filename->d_name;
		fileDestPath += filename->d_name;
		if (IsDir(fileSourceFath.c_str()))
		{
			if (strncmp(filename->d_name, ".", 1) && strncmp(filename->d_name, "..", 2))
			{
				CopyFolder(fileSourceFath, fileDestPath);
			}
		}
		else
		{
			CopyFile(fileSourceFath, fileDestPath);
			printf("Copy From %s To %s Successed\n", fileSourceFath.c_str(), fileDestPath.c_str());
		}
	}
}
//
//int main(int argc, char *argv[])
//{
//	if (argv[1] == NULL || argv[2] == NULL)
//	{
//		printf("Please Input Source Path and Destnation Path\n");
//		return 1;
//	}
//	std::string sourcePath = argv[1];//source path
//	std::string destPath = argv[2];//destnation path
//	DIR* source = opendir(sourcePath.c_str());
//	DIR* destination = opendir(destPath.c_str());
//	if (!source)
//	{
//		printf("Source Dir Path Is Not Existed\n");
//		return 1;
//	}
//	if (!destination)
//	{
//		printf("Destnation Dir Path Is Not Existed\n");
//	}
//	CopyFolder(sourcePath, destPath);
//
//	closedir(source);
//	closedir(destination);
//	return 0;
//}


std::string GetSysFormatTime()
{
	time_t cur_time = time(NULL);
	struct tm local_tm;

	if (NULL == localtime_r(&cur_time, &local_tm))
	{
		return "";
	}

	char stm[20] = { 0 };
	sprintf(stm, "%4d%02d%02d%02d%02d%02d", local_tm.tm_year + 1900,
		local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
	return stm;
}
