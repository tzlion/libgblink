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
