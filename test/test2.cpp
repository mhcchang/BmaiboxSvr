#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <thread>

typedef struct cpu_occupy_          //����һ��cpu occupy�Ľṹ��
{
	char name[20];                  //����һ��char���͵�������name��20��Ԫ��
	unsigned int user;              //����һ���޷��ŵ�int���͵�user
	unsigned int nice;              //����һ���޷��ŵ�int���͵�nice
	unsigned int system;            //����һ���޷��ŵ�int���͵�system
	unsigned int idle;              //����һ���޷��ŵ�int���͵�idle
	unsigned int iowait;
	unsigned int irq;
	unsigned int softirq;
}cpu_occupy_t;

double cal_cpuoccupy(cpu_occupy_t *o, cpu_occupy_t *n)
{
	double od, nd;
	double id, sd;
	double cpu_use;

	od = (double)(o->user + o->nice + o->system + o->idle + o->softirq + o->iowait + o->irq);//��һ��(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od
	nd = (double)(n->user + n->nice + n->system + n->idle + n->softirq + n->iowait + n->irq);//�ڶ���(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od

	id = (double)(o->idle);    //�û���һ�κ͵ڶ��ε�ʱ��֮���ٸ���id
	sd = (double)(n->idle);    //ϵͳ��һ�κ͵ڶ��ε�ʱ��֮���ٸ���sd
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
	sleep(1);
	//�ڶ��λ�ȡcpuʹ�����
	get_cpuoccupy((cpu_occupy_t *)&cpu_stat2);

	//����cpuʹ����
	cpu = cal_cpuoccupy((cpu_occupy_t *)&cpu_stat1, (cpu_occupy_t *)&cpu_stat2);

	return cpu;
}

int main4(int argc, char **argv)
{
	//std::thread([]() {
	//	unsigned int i = 0;
	//	while (1) {
	//		i = i + 1;
	//		//usleep(1);
	//	}
	//}).detach();
	//std::thread([]() {
	//	unsigned int i = 0;
	//	while (1) {
	//		i = i + 1;
	//		//usleep(1);
	//	}
	//}).detach();
	while (1)
	{
		auto usage = get_sysCpuUsage();
		printf("CPUռ����:%f\n", usage);
		//sleep(1);
	}

	printf("keypress \n");
	getchar();
	return 0;
}


int GetShellRes(char* szCmd, std::stringstream &sstream)
{
    FILE * stream;
    FILE * wstream;
    char buf[1024] ={ 0};
    int streamSize;
    memset( buf, 0, sizeof(buf) );
    stream = popen(szCmd, "r" ); 
    streamSize = 0;
    char* tmp = NULL;
	
    while  (  (tmp = fgets(buf, sizeof(buf), stream) )!= NULL)
    {
		sstream << tmp;
		printf("GetShellRes %s \n", buf);
		streamSize += strlen(tmp);
	} 
    
    pclose( stream );
    return streamSize;
}

int main1()
{
    using namespace std;
    string str;
    char iface[200];
    stringstream ss;

	int nn = GetShellRes("df -m /", ss);
	printf("GetShellRes %d \n", nn);

                getline(ss, str, '\n');
	printf("getline 1 %s\n", str.c_str());
                int ii = 0;
                getline(ss, str, '\n');
	printf("getline 2 %s\n", str.c_str());
	unsigned long Size, Used, Avail;
	if (sscanf(str.c_str(), "%s\t%d\t%d\t%d", iface, &Size, &Used, &Avail) != 4)
		printf("error \n");
	else
		printf("%s\t%d\t%d\t%d", iface, Size, Used, Avail);

	getchar();
     return 0;
}

int main5()
{
	using namespace std;
	string str;
	char iface[200];
	stringstream ss;

	int nn = GetShellRes("bm-smi --file=./smi.txt --lms=300", ss);

	usleep(3000);
	system("clear");
	printf("GetShellRes %d \n", nn);


	//getchar();
	return 0;
}

int main2(void)
{
    FILE *fp = NULL;
    char buf[1000]={0};
    fp = popen("df -m /", "r");
    if(fp) {
    
        int ret =  fread(buf,1,sizeof(buf)-1,fp);
        if(ret > 0) {
            printf("%s",buf);
        }
        pclose(fp);
        printf("");
    }
	return 0;
}

int ccc()
{
	char* szCmd = "cat /etc/network/interfaces.d/eth0";
	std::stringstream sstream;
	GetShellRes(szCmd, sstream);
	char iface[200];
	//auto eth0
	//	iface eth0 inet static
	//	address 192.168.1.92
	//	netmask 255.255.255.0
	//	gateway 192.168.1.1
	//	dns-nameservers 192.168.1.1
	using namespace std;
	string str;
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');
	if (sscanf(str.c_str(), "%s\t%s", iface, &ip) != 2)
		return -1;
	if (sscanf(str.c_str(), "%s\t%s", iface, &mask) != 2)
		return -2;
	if (sscanf(str.c_str(), "%s\t%s", iface, &gateway) != 2)
		return -3;
	if (sscanf(str.c_str(), "%s\t%s", iface, &dns) != 2)
		return -4;
	return 0;
}

int main(void0)
{

}