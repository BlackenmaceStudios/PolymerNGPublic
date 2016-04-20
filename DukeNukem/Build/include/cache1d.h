// cache1d.h

#ifndef cache1d_h_
#define cache1d_h_

#include "compat.h"
#include "mutex.h"

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WITHKPLIB
	int32_t cache1d_file_fromzip(int32_t fil);
#endif

	extern char *kpzbuf;
	extern int32_t kpzbufsiz;
	extern int32_t kpzbufloadfil(int32_t);
	extern int32_t kpzbufload(const char *);

	void	initcache(intptr_t dacachestart, int32_t dacachesize);
	void	allocache(intptr_t *newhandle, int32_t newbytes, char *newlockptr);
	void	agecache(void);

	extern int32_t pathsearchmode;	// 0 = gamefs mode (default), 1 = localfs mode (editor's mode)
	char *listsearchpath(int32_t initp);
#define addsearchpath(a) addsearchpath_user(a, 0)
	int32_t     addsearchpath_user(const char *p, int32_t user);
	int32_t     removesearchpath(const char *p);
	void     removesearchpaths_withuser(int32_t usermask);
	int32_t		findfrompath(const char *fn, char **where);
	int32_t     openfrompath(const char *fn, int32_t flags, int32_t mode);
	BFILE  *fopenfrompath(const char *fn, const char *mode);

	int32_t	initgroupfile(const char *filename);
	void	uninitgroupfile(void);
	int32_t	kopen4load(const char *filename, char searchfirst);	// searchfirst: 0 = anywhere, 1 = first group, 2 = any group
	int32_t	kread(int32_t handle, void *buffer, int32_t leng);
#define kread_and_test(handle, buffer, leng) EDUKE32_PREDICT_FALSE(kread((handle), (buffer), (leng)) != (leng))
	int32_t	klseek(int32_t handle, int32_t offset, int32_t whence);
#define klseek_and_test(handle, offset, whence) EDUKE32_PREDICT_FALSE(klseek((handle), (offset), (whence)) < 0)
	int32_t	kfilelength(int32_t handle);
	int32_t	ktell(int32_t handle);
	void	kclose(int32_t handle);

	//
	// BuildFile
	//
	class BuildFile
	{
	public:
		~BuildFile();

		enum BuildFileOpenType
		{
			BuildFile_Read,
			BuildFile_Write,
			BuildFile_Append
		};

		static BuildFile *OpenFile(const char *fmt, BuildFileOpenType openType);

		static bool WriteFile(const char *fileName, const char *buffer, int length);

		// Returns true if the file is loaded.
		bool          isLoaded() { return fp >= 0; }

		// Returns the current position in the file.
		int32_t		  Tell();

		// Returns the length of the file.
		int32_t		  Length();

		// Seeks in the file.
		void		Seek(int32_t offset, int32_t whence);

		// Reads a buffer from a file.
		int32_t		  Read(void *buffer, int32_t leng);

		void				ReadInt(int &value);
		void				ReadUnsignedInt(unsigned int &value);
		void				ReadShort(short &value);
		void				ReadUnsignedShort(unsigned short &value);
		void				ReadChar(char &value);
		void				ReadUnsignedChar(unsigned char &value);
		void				ReadFloat(float &value);
		void				ReadBool(bool &value);
		void				ReadString(std::string &str);

		void				Write(const char *buffer, int length);
		void				WriteInt(const int value);
		void				WriteUnsignedInt(const unsigned int value);
		void				WriteShort(const short value);
		void				WriteUnsignedShort(unsigned short value);
		void				WriteChar(const char value);
		void				WriteUnsignedChar(const unsigned char value);
		void				WriteFloat(const float value);
		void				WriteBool(const bool value);
		void				WriteString(const char *string);
	private:
		BuildFile(int32_t fp, BuildFileOpenType openType);
		BuildFile(FILE *file, BuildFileOpenType openType);
	private:
		int32_t	fp;
		FILE *file;
		BuildFileOpenType openType;
	};

	//
	// kreadfile
	__forceinline char    *kreadfile(const char *filename, uint32_t &flen)
	{
		int32_t fp;
		char *tx;

		fp = kopen4load(filename, 0);
		if (fp < 0)
			return NULL;

		flen = kfilelength(fp);
		tx = (char *)Xmalloc(flen + 2);

		kread(fp, tx, flen);
		tx[flen] = tx[flen + 1] = 0;

		kclose(fp);

		return tx;
	}

	void krename(int32_t crcval, int32_t filenum, const char *newname);

	typedef struct { intptr_t *hand; int32_t leng; char *lock; } cactype;

	enum {
		CACHE1D_FIND_FILE = 1,
		CACHE1D_FIND_DIR = 2,
		CACHE1D_FIND_DRIVE = 4,
		CACHE1D_FIND_NOCURDIR = 8,

		CACHE1D_OPT_NOSTACK = 0x100,

		// the lower the number, the higher the priority
		CACHE1D_SOURCE_DRIVE = 0,
		CACHE1D_SOURCE_CURDIR = 1,
		CACHE1D_SOURCE_PATH = 2,	// + path stack depth
		CACHE1D_SOURCE_ZIP = 0x7ffffffe,
		CACHE1D_SOURCE_GRP = 0x7fffffff,
	};
	typedef struct _CACHE1D_FIND_REC {
		char *name;
		int32_t type, source;
		struct _CACHE1D_FIND_REC *next, *prev, *usera, *userb;
	} CACHE1D_FIND_REC;
	void klistfree(CACHE1D_FIND_REC *rec);
	CACHE1D_FIND_REC *klistpath(const char *path, const char *mask, int32_t type);

	int32_t	kdfread(void *buffer, bsize_t dasizeof, bsize_t count, int32_t fil);
#if 0
	int32_t	dfread(void *buffer, bsize_t dasizeof, bsize_t count, BFILE *fil);
	void	kdfwrite(const void *buffer, bsize_t dasizeof, bsize_t count, int32_t fil);
#endif
	void	dfwrite(const void *buffer, bsize_t dasizeof, bsize_t count, BFILE *fil);

#ifdef __cplusplus
}
#endif

#endif // cache1d_h_
