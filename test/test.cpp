// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <document.h>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include <fstream>
#include <experimental/filesystem>
#include "StringBuffer.h"
#include "PrettyWriter.h"
#include "OStreamWrapper.h"

#define LOG4QT_VERSION_MAJOR "1"
#define LOG4QT_VERSION_MINOR "4"
#define LOG4QT_VERSION_PATCH "2"
#define STR(x, y, z) x.y.z


//#define LOG4QT_VERSION_STR STR(LOG4QT_VERSION_MAJOR).STR(LOG4QT_VERSION_MINOR).STR(LOG4QT_VERSION_PATCH)
#define LOG4QT_VERSION_STR STR(LOG4QT_VERSION_MAJOR, LOG4QT_VERSION_MINOR, LOG4QT_VERSION_PATCH)
#define LOG4QT_VERSION_STR1 LOG4QT_VERSION_MAJOR "." LOG4QT_VERSION_MINOR "." LOG4QT_VERSION_PATCH

#include "logger.h"

void sss()
{
	printf("LOG4QT_VERSION_STR1 %s", LOG4QT_VERSION_STR1);
	system("pause");
}

void Save()
{
	std::string content = "{ \"node1\": \"n1\", \"node2\": {\"n2_1\":1}}";

	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	if (doc.HasParseError())
	{
		printf("rapidjson Parse error 1\n");
		return ;
	}
	if (!doc.IsObject())
	{
		printf("rapidjson Parse error 2\n");
		return;
	}
	if (!doc.HasMember("node2") || !doc["node2"].IsObject())
	{
		printf(" node 3\n");
		return ;
	}

	std::ofstream ofs("d:\\aaa.json");
	if (!ofs.is_open())
		return;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document docsave;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	std::string boxId = "dsfsdfsd";
	//doc.AddMember("node1", 10, allocator);
	doc["node1"].SetString(boxId.c_str(), boxId.length());
	doc["node2"].AddMember("n2_3", 5, allocator);
	doc.Accept(writer);
}

#include <regex>
#include <sstream>
#include <iostream>
void ref2()
{
	using namespace std;

	regex pattern("\\d+");	/*匹配一个到无穷个数字*/
	string s = "51x41+(5-13/2)x3a";
	smatch result;

	string data = "001-Neo,002-Lucia";
	
	regex reg("00(\\d+)-(\\w+)");

	string::const_iterator iter_begin = data.cbegin();
	string::const_iterator iter_end = data.cend();
	//while (regex_search(iter_begin, iter_end, result, pattern))
	while (regex_search(iter_begin, iter_end, result, reg))
	{
		cout << "查找成功：" << result[0] << endl;
		cout << "查找结果子串的在源串中的迭代器位置" << *result[0].first << endl;
		cout << "查找结果子串的在源串后面的位置" << *result[0].second << endl;
		iter_begin = result[0].second;	//更新迭代器位置
	}

	char datac[] = "001-Neo,002-Lucia";
	//regex reg("(\\d+)-(\\w+)");
	regex regx("001-(\\w+),");
	cout << regex_replace(datac, regx, "$1 name=$2");
	std::string res = regex_replace(datac, regx, "$1");
	cout << res;
}

//void ref3() err
//{
//	using namespace std;
//	string str = "Hello_2018";
//	smatch result;
//	regex pattern("(.{5})_(\\d{4})"); //匹配5个任意单字符 + 下划线 + 4个数字
//
//	string data1 = "001-Neo,002-Lucia";
//	regex reg("(\\d+)-(\\w+)");
//	regex reg1("001-(\\w+),");
//	//if (regex_match(str, result, reg))
//	//if (regex_match(data1, result, reg1))
//	{
//		cout << result[0] << endl; //完整匹配结果，Hello_2018
//		cout << result[1] << endl; //第一组匹配的数据，Hello
//		cout << result[2] << endl; //第二组匹配的数据，2018
//		cout << "结果显示形式2" << endl;
//		cout << result.str() << endl; //完整结果，Hello_2018
//		cout << result.str(1) << endl; //第一组匹配的数据，Hello
//		cout << result.str(2) << endl; //第二组匹配的数据，2018
//	}
//}

void ref1()
{
	std::string szt = "I2C port 0, addr 0x5c, type 0x3, reg 0x0, value 0x0"
		"ISL68127 revision 0x33"
		"ISL68127 switch to output 0, ret = 0"
		"ISL68127 output voltage : 615mV"
		"ISL68127 output current : 2900mA"
		"ISL68127 temperature 1 : 37`C"
		"ISL68127 output power : 2W" // → TPU
		"ISL68127 switch to output 1, ret = 0"
		"ISL68127 output voltage : 876mV"
		"ISL68127 output current : 1700mA"
		"ISL68127 temperature 1 : 38`C"
		"ISL68127 output power : 134W"; // → CPU / Video"

	std::string sz = "#server 127.127.8.1 mode 135 prefer \n"
		"   server 127";
	//std::stringstream sstream;
	//sstream << sz;
	std::smatch what;
	std::smatch result;
	std::cmatch m;
	//std::string pattern{ "ISL68127 output voltage : [0-9]+mV" };
	std::string pattern{ "output voltage : [0-9]+" };
	std::regex re(pattern);
	std::regex reg1("addr 0x[0-9]+");
	//for (std::string tmp; ;)
	{
		//what.prefix
		//sstream >> tmp;
		bool ret = std::regex_search(sz, what, re);

		std::regex reg2("[^#][\s]{0,}server");

		ret = std::regex_search(sz, what, reg2);

		std::string res = regex_replace(sz, reg2, "$1");
		//std::regex reg3("output power : [0-9]{1,}W[\n]{0,}ISL68127");
		std::regex reg3("output power : (\\d+)W");

		
		std::regex reg4("temperature 1 : (\\d+)`C");
		ret = std::regex_search(szt, what, reg3);

		using namespace std;
		string::const_iterator iter_begin = szt.cbegin();
		string::const_iterator iter_end = szt.cend();
		float ff = 0.0f;

		std::string szccc = "23344";
		ff = atof(szccc.c_str());
		while (std::regex_search(iter_begin, iter_end, result, reg3) )
		{
			cout << "查找成功：" << result[0] << " match =" << result[1] << endl;
			//cout << "查找结果子串的在源串中的迭代器位置" << *result[0].first << endl;
			//cout << "查找结果子串的在源串后面的位置" << *result[0].second << endl;
			
			ff = atof(result[1].str().c_str());
			iter_begin = result[0].second;	//更新迭代器位置
		}
		ret = std::regex_search(szt, what, reg4);
		//ret = std::regex_match(sz, m, re);

		//ret = std::regex_match(sz, what, re);
		//ret = std::regex_match(sz.c_str(), m, reg1);
		//std::smatch what;

		for (int i = 0; i < what.size(); ++i)
		{
			if (what[i].matched)
			{
				std::cout << what[i] << std::endl;
				std::string res = what[i].str();
			}
		}

		if (ret)
			fprintf(stderr, "%s, can match\n", what);
		else
			fprintf(stderr, "%s, can not match\n", what);
	}
}


#include <iostream>
#include <regex>
#include <string>
#include <iterator>

using namespace std;

int main1()
{
	system("COLOR 1F");

	cout << "==========regex_search=========" << endl;
	string srcStr("---ISMILE--2019-01-24--ISMILE01-THURSDAY");
	std::smatch m;
	std::regex e("\\d{4}-\\d{2}-\\d{2}");
	if (std::regex_search(srcStr, m, e, regex_constants::match_default))
	{
		for (auto iter : m)
		{
			cout << iter << endl;
		}
		string tempStr = m.suffix().str();
		cout << "tempStr:" << tempStr << endl;
		string mbeginStr = *m.begin();
		cout << "mbeginStr:" << mbeginStr << endl;
		string mEndStr = *(m.end() - 1);
		cout << "mEndStr:" << mEndStr << endl;
		int length = m.size();
		cout << "length:" << length << endl;
		cout << "m.prefix():" << m.prefix().str() << endl;
	}

	cout << "========regex_match=======" << endl;
	const char *strch = srcStr.c_str();
	std::cmatch cm;
	std::regex ce("(.*)(ISMILE)(.*)");
	if (std::regex_match(strch, cm, ce, regex_constants::match_default))
	{
		cout << "cm.size=" << cm.size() << endl;
		for (auto iter : cm)
		{
			cout << iter << endl;
		}
	}

	cout << "========regex_replace=======" << endl;
	string tmpStr("this is ISMILELI's test for regex!");
	std::regex eStr("(ISMILE)");
	std::string result;
	// 此处改成$2时结果打印是"this is LI's test for regex!"，可以参考测试代码中的result3的打印结果多方寻找答案不得，这个地方的参数传值[官方的描述](http://www.cplusplus.com/reference/regex/regex_replace/)是大于0的不超
	//过两位数的整数
	std::regex_replace(std::back_inserter(result), tmpStr.begin(), tmpStr.end(), eStr, "$1");
	cout << "result:" << result << endl;

	string tmpStr2("ISMILE,ISMILELI,SMILE");
	std::regex eStr2("(ISMILE)");
	std::string result2;
	std::regex_replace(std::back_inserter(result2), tmpStr2.begin(), tmpStr2.end(), eStr2, "lisa-$1");
	cout << "result2:" << result2 << endl;
	std::string result3;
	std::regex_replace(std::back_inserter(result3), tmpStr2.begin(), tmpStr2.end(), eStr2, "lisa$2");
	cout << "result3:" << result3 << endl;
	return 0;
}

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

void ccc() 
{
	using namespace std;
	string str;
	string str_cin("one#two#three\nfour#five");
	stringstream ss;
	ss << str_cin;
	int ii = 0;
	while (getline(ss, str, '\n'))
	{
		cout << "ii " << ii++ << endl;
		cout << str << endl;
	}
	getchar();
}

void ddd()
{
	std::string aa = "10:33:22 up 15 min,  1 users";
	int dd = 0;
	char buf1[255];
	char buf2[255];
	char buf3[255];
	char buf4[255];

	string bb = "0123abcABC";
	sscanf(bb.c_str(), "%[0-9]%[a-z]%[A-Z]", buf1, buf2, buf3);

	string cc = "10:33:22 up 15 min";
	sscanf(cc.c_str(), "%* up %[0-9] min", buf1, buf2);
	//printf("7.string=%s\n", string);
	printf("7.buf1=%s, buf2=%s\n\n", buf1, buf2);
	//给定一个字符串 iios/12DDWDFF@122，获取 / 和 @ 之间的字符串，
	sscanf(" iios/12DDWDFF@122", "%*[^/]/%[^@]",  buf1);

	int iii = sscanf(" 00:30:04 up  1:24,  2 users,  load average: 2.16, 2.06, 2.01", "%s%s%d:%d,", buf1, buf2, buf3, buf4);
	printf("%d", iii);

	if (sscanf(aa.c_str(), "%s%s%d%s", buf1, buf2, buf3, buf4) != 4)
	{
		perror("err! \n");
	}
}
void GStringseparation4(const char *sor, unsigned char *arr)
{
	int Arr[4];
	char buf1[255];
	sscanf(sor, "%s%d.%d.%d.%d", buf1, &Arr[0], &Arr[1], &Arr[2], &Arr[3]);
	for (char i = 0;i<4;i++)
		arr[i] = Arr[i];
}

int eee()
{
	unsigned char arr[4];
	GStringseparation4("        address 192.168.1.92", arr);
	for (char i = 0;i<4;i++)
		printf("%d-", arr[i]);
	return 0;
}

void fff()
{
	std::string body = "{\"channel\":0}";
	int ch = -1;

	if (sscanf(body.c_str(), "%s%d", ch) != 1)
	{
		std::string res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		printf("erro");

	}
	else
	{
		printf("ch=%d", ch);
	}
}
int main()
{
	MHC_LOGGER_INIT("testlog.LOG", MHC_SINK_ROTATING_FILE, MHC_LEVEL_DEBUG);
	//MHC_DEBUG("[MQClient] publishImageData: detectMsgId={}, extractAsync={}", 2, 4);

	//sss();
	//Save();
	//ref1();
	//ref2();
	//ccc();
	//ddd();
	fff();
	//main1();
    std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

