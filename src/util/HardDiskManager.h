#if _MSC_VER > 1000
#pragma once
#endif

#include "defs.h"

//#include "sys_inc.h"
#if __WINDOWS_OS__
#include <windows.h>
#include <string>
class WHardDiskManager
{
public:
	WHardDiskManager();
	virtual ~WHardDiskManager();

	bool CheckFreeSpace(LPCTSTR lpDirectoryName);

	DWORD64 GetFreeBytesAvailable(void);
	DWORD64 GetTotalNumberOfBytes(void);
	DWORD64 GetTotalNumberOfFreeBytes(void);

	double GetFreeGBytesAvailable(void);
	double GetTotalNumberOfGBytes(void);
	double GetTotalNumberOfFreeGBytes(void);
	double GetFreeMBytesAvailable(void);
	double GetTotalNumberOfMBytes(void);
	double GetTotalNumberOfFreeMBytes(void);

	double GetFreeAvailableOfPercentage(void);
	double GetTotalFreeOfPercentage(void);
private:
	ULARGE_INTEGER m_uliFreeBytesAvailable;
	ULARGE_INTEGER m_uliTotalNumberOfBytes;
	ULARGE_INTEGER m_uliTotalNumberOfFreeBytes;
};

#elif __LINUX_OS__
#include <sys/statfs.h>  
#include <stdio.h>  
#include <string>
/*
* 文件系统很多，列表如下
ADFS_SUPER_MAGIC      0xadf5
AFFS_SUPER_MAGIC      0xADFF
BEFS_SUPER_MAGIC      0x42465331
BFS_MAGIC             0x1BADFACE
CIFS_MAGIC_NUMBER     0xFF534D42
CODA_SUPER_MAGIC      0x73757245
COH_SUPER_MAGIC       0x012FF7B7
CRAMFS_MAGIC          0x28cd3d45
DEVFS_SUPER_MAGIC     0x1373
EFS_SUPER_MAGIC       0x00414A53
EXT_SUPER_MAGIC       0x137D
EXT2_OLD_SUPER_MAGIC 0xEF51
EXT2_SUPER_MAGIC      0xEF53
EXT3_SUPER_MAGIC      0xEF53
HFS_SUPER_MAGIC       0x4244
HPFS_SUPER_MAGIC      0xF995E849
HUGETLBFS_MAGIC       0x958458f6
ISOFS_SUPER_MAGIC     0x9660
JFFS2_SUPER_MAGIC     0x72b6
JFS_SUPER_MAGIC       0x3153464a
MINIX_SUPER_MAGIC     0x137F
MINIX_SUPER_MAGIC2    0x138F
MINIX2_SUPER_MAGIC    0x2468
MINIX2_SUPER_MAGIC2   0x2478
MSDOS_SUPER_MAGIC     0x4d44
NCP_SUPER_MAGIC       0x564c
NFS_SUPER_MAGIC       0x6969
NTFS_SB_MAGIC         0x5346544e
OPENPROM_SUPER_MAGIC 0x9fa1
PROC_SUPER_MAGIC      0x9fa0
QNX4_SUPER_MAGIC      0x002f
REISERFS_SUPER_MAGIC 0x52654973
ROMFS_MAGIC           0x7275
SMB_SUPER_MAGIC       0x517B
SYSV2_SUPER_MAGIC     0x012FF7B6
SYSV4_SUPER_MAGIC     0x012FF7B5
TMPFS_MAGIC           0x01021994
UDF_SUPER_MAGIC       0x15013346
UFS_MAGIC             0x00011954
USBDEVICE_SUPER_MAGIC 0x9fa2
VXFS_SUPER_MAGIC      0xa501FCF5
XENIX_SUPER_MAGIC     0x012FF7B4
XFS_SUPER_MAGIC       0x58465342
_XIAFS_SUPER_MAGIC    0x012FD16D
*/

typedef enum
{
	DISK_FORMAT_FAT, DISK_FORMAT_NTFS, DISK_FORMAT_EXT2, DISK_FORMAT_UNKNOW//其他省略
} DISK_FORMAT;

class WHardDiskManager
{
public:
	/*
	* 设置磁盘目录
	*/
	WHardDiskManager(const std::string& _path = "/");

	virtual ~WHardDiskManager();

	unsigned long long  GetFreeBytesAvailable(void);
	unsigned long long  GetTotalNumberOfBytes(void);
	unsigned long long  GetTotalNumberOfFreeBytes(void);

	double GetFreeGBytesAvailable(void);
	double GetTotalNumberOfGBytes(void);
	double GetTotalNumberOfFreeGBytes(void);
	double GetFreeMBytesAvailable(void);
	double GetTotalNumberOfMBytes(void);
	double GetTotalNumberOfFreeMBytes(void);

	double GetFreeAvailableOfPercentage(void);
	double GetTotalFreeOfPercentage(void);

	/*
	* 刷新磁盘信息
	* 当磁盘信息发生改变是应当刷新后再获取
	*/
	void refreshInfo();

	/*
	* 获取磁盘总大小
	* 单位为B
	*/
	long long getTotalSize();

	/*
	* 获取磁盘已使用大小
	* 单位为B
	*/
	long long getUsedSize();

	/*
	* 获取磁盘已使用大小
	* 单位为B
	*/
	long long getLeftSize();

	/*
	* 获取磁盘文件系统
	*/
	DISK_FORMAT getDiskFormat();

private:
	long long disk_total_capacity;
	long long disk_used_capacity;
	long long disk_free_capacity;
	DISK_FORMAT disk_format;
	std::string path;
};

#endif
