// HardDiskManager.cpp

#include "HardDiskManager.h"
#if __WINDOWS_OS__
//#include <afxwin.h>

WHardDiskManager::WHardDiskManager()
{
	m_uliFreeBytesAvailable.QuadPart = 0L;
	m_uliTotalNumberOfBytes.QuadPart = 0L;
	m_uliTotalNumberOfFreeBytes.QuadPart = 0L;
}

WHardDiskManager::~WHardDiskManager()
{
}

bool WHardDiskManager::CheckFreeSpace(LPCTSTR lpDirectoryName)
{
	if (!GetDiskFreeSpaceEx(
		lpDirectoryName,
		&m_uliFreeBytesAvailable,
		&m_uliTotalNumberOfBytes,
		&m_uliTotalNumberOfFreeBytes))
		return false;

	return true;
}

DWORD64 WHardDiskManager::GetFreeBytesAvailable(void)
{
	return m_uliFreeBytesAvailable.QuadPart;
}

DWORD64 WHardDiskManager::GetTotalNumberOfBytes(void)
{
	return m_uliTotalNumberOfBytes.QuadPart;
}

DWORD64 WHardDiskManager::GetTotalNumberOfFreeBytes(void)
{
	return m_uliTotalNumberOfFreeBytes.QuadPart;
}

double WHardDiskManager::GetFreeGBytesAvailable(void)
{
	return (double)((signed __int64)(m_uliFreeBytesAvailable.QuadPart) / 1073741824);
}

double WHardDiskManager::GetTotalNumberOfGBytes(void)
{
	return (double)((signed __int64)(m_uliTotalNumberOfBytes.QuadPart) / 1073741824);
}

double WHardDiskManager::GetTotalNumberOfFreeGBytes(void)
{
	return (double)((signed __int64)(m_uliTotalNumberOfFreeBytes.QuadPart) / 1073741824);
}

double WHardDiskManager::GetFreeMBytesAvailable(void)
{
	return (double)((signed __int64)(m_uliFreeBytesAvailable.QuadPart) / 1048576);
}

double WHardDiskManager::GetTotalNumberOfMBytes(void)
{
	return (double)((signed __int64)(m_uliTotalNumberOfBytes.QuadPart) / 1048576);
}

double WHardDiskManager::GetTotalNumberOfFreeMBytes(void)
{
	return (double)((signed __int64)(m_uliTotalNumberOfFreeBytes.QuadPart) / 1048576);
}

double WHardDiskManager::GetFreeAvailableOfPercentage(void)
{
	return (double)((double)m_uliFreeBytesAvailable.QuadPart / m_uliTotalNumberOfBytes.QuadPart * 100) ;
}

double WHardDiskManager::GetTotalFreeOfPercentage(void)
{
	return (double)((double)m_uliTotalNumberOfFreeBytes.QuadPart / m_uliTotalNumberOfBytes.QuadPart * 100);
}

#elif __LINUX_OS__

#include <sys/vfs.h>
#include <string>

WHardDiskManager::WHardDiskManager(const std::string& _path) :
	disk_total_capacity(0), disk_used_capacity(0), disk_free_capacity(0), disk_format(
		DISK_FORMAT_UNKNOW), path(_path) 
{
	struct statfs buf;
	int i = statfs(path.c_str(), &buf);
	if (i < 0) {
		//printf("get disk infomation faild\n");
	}
	else {
		switch (buf.f_type) {
		case 0x4d44:
			disk_format = DISK_FORMAT_FAT;
			break;
		case 0x5346544e:
		case 0X65735546:
			disk_format = DISK_FORMAT_NTFS;
			break;
		case 0xEF53:
		case 0xEF51:
			disk_format = DISK_FORMAT_EXT2;
			break;
		default:
			disk_format = DISK_FORMAT_UNKNOW;
			break;
		}
		disk_total_capacity = (((long long)buf.f_bsize
			* (long long)buf.f_blocks));
		disk_free_capacity =
			(((long long)buf.f_bsize * (long long)buf.f_bfree));
		disk_used_capacity = disk_total_capacity - disk_free_capacity;
	}
}

WHardDiskManager::~WHardDiskManager() 
{
}

void WHardDiskManager::refreshInfo() 
{
	struct statfs buf;
	int i = statfs(path.c_str(), &buf);
	if (i < 0) {
		//printf("refresh get disk infomation faild\n");
		return;
	}
	switch (buf.f_type) {
	case 0x4d44:
		disk_format = DISK_FORMAT_FAT;
		break;
	case 0x5346544e:
	case 0X65735546:
		disk_format = DISK_FORMAT_NTFS;
		break;
	case 0xEF53:
	case 0xEF51:
		disk_format = DISK_FORMAT_EXT2;
		break;
	default:
		disk_format = DISK_FORMAT_UNKNOW;
		break;
	}
	disk_total_capacity = (((long long)buf.f_bsize * (long long)buf.f_blocks));
	disk_free_capacity = (((long long)buf.f_bsize * (long long)buf.f_bfree));
	disk_used_capacity = disk_total_capacity - disk_free_capacity;
}

long long WHardDiskManager::getTotalSize() 
{
	return disk_total_capacity;
}

long long WHardDiskManager::getUsedSize() 
{
	return disk_used_capacity;
}

long long WHardDiskManager::getLeftSize() 
{
	return disk_free_capacity;
}

DISK_FORMAT WHardDiskManager::getDiskFormat() 
{
	return disk_format;
}

unsigned long long WHardDiskManager::GetFreeBytesAvailable(void)
{
	return disk_free_capacity;
}

unsigned long long  WHardDiskManager::GetTotalNumberOfBytes(void)
{
	return disk_total_capacity;
}

unsigned long long  WHardDiskManager::GetTotalNumberOfFreeBytes(void)
{
	return disk_free_capacity;
}

double WHardDiskManager::GetFreeGBytesAvailable(void)
{
	return (double)((double)disk_free_capacity / 1073741824);
}

double WHardDiskManager::GetTotalNumberOfGBytes(void)
{
	return (double)((double)disk_total_capacity / 1073741824);
}

double WHardDiskManager::GetTotalNumberOfFreeGBytes(void)
{
	return (double)(disk_free_capacity / 1073741824);
}

double WHardDiskManager::GetFreeMBytesAvailable(void)
{
	return (double)((double)disk_free_capacity / 1048576);
}

double WHardDiskManager::GetTotalNumberOfMBytes(void)
{
	return (double)((double)disk_total_capacity / 1048576);
}

double WHardDiskManager::GetTotalNumberOfFreeMBytes(void)
{
	return (double)((double)disk_free_capacity / 1048576);
}

double WHardDiskManager::GetFreeAvailableOfPercentage(void)
{
	return (double)((double)disk_free_capacity / disk_total_capacity * 100);
}

double WHardDiskManager::GetTotalFreeOfPercentage(void)
{
	return (double)((double)disk_free_capacity / disk_total_capacity * 100);
}
#endif
