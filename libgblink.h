#define GBLINK_EXPORTS

#ifdef GBLINK_EXPORTS
#define GBLINK_API __declspec(dllexport)
#else
#define GBLINK_API __declspec(dllimport)
#endif

#define GBLINK_CALL __cdecl

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char U8;
typedef unsigned short U16;

#define LPTREG_DATA 0x378
#define LPTREG_STATUS (LPTREG_DATA + 1)
#define LPTREG_CONTROL (LPTREG_DATA + 2)

#define STATUS_BUSY 0x80
#define CTL_MODE_DATAIN 0x20
#define D_CLOCK_HIGH 0x02

typedef void(*LinkerLogger)(const char*);

GBLINK_API bool GBLINK_CALL InitLinker();
GBLINK_API void GBLINK_CALL DeinitLinker();
GBLINK_API void GBLINK_CALL ReadBlock(U8 * dest, U16 addr, int len);
GBLINK_API U8 GBLINK_CALL ReadByte(U16 addr);
GBLINK_API void GBLINK_CALL WriteByte(U16 addr, U8 val);
GBLINK_API void GBLINK_CALL SetLogger(LinkerLogger newlogger);
GBLINK_API bool GBLINK_CALL IsLinkerActive();
GBLINK_API U8* GBLINK_CALL GetBank0();

#ifdef __cplusplus
}
#endif
