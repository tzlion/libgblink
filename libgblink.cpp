/*
  SIMPLIFIED GBLINK IMPLEMENTATION
  Based on: original gblinkdl by Brian Provinciano
  Modified gblinkdx by taizou
  (this file is not strictly GPL it's just whatever sorry)
*/

#include "libgblink.h"

#include "stdio.h"
#include "string.h"

#include "windows.h"

using namespace std;

U8 bank0[0x4000];
bool linkerActive = false;
bool linkerInitialising = false;
LinkerLogger logger = nullptr;
HINSTANCE hInpOutDll;

typedef void(__stdcall *lpOut32)(short, short);
typedef short(__stdcall *lpInp32)(short);
lpOut32 gfpOut32;
lpInp32 gfpInp32;

char msg[420];

void logMessage(const char* message) {
    if (logger) {
        logger(message);
    }
}

unsigned char inportb(unsigned short port)
{
    return gfpInp32(port);
}

void outportb(unsigned short port, unsigned char value)
{
    gfpOut32(port,value);
}

void lptdelay(int amt)
{
    for(int i=0;i<amt;i++)
        inportb(LPTREG_DATA);
}

U8 gb_sendbyte(U8 value)
{
    U8 read = 0;
    for(int i=7;i>=0;i--) {
        U8 v = (value>>i)&1;

        outportb(LPTREG_DATA, v|D_CLOCK_HIGH);
        outportb(LPTREG_DATA, v);

        U8 stat = inportb(LPTREG_STATUS);

        if(!(stat&STATUS_BUSY))
            read |= (1<<i);

        outportb(LPTREG_DATA, v|D_CLOCK_HIGH);
    }
    lptdelay(64);
    return read;
}

U8 gb_readbyte()
{
    U8 read = 0;
    for(int i=7;i>=0;i--) {
        outportb(LPTREG_DATA, D_CLOCK_HIGH);
        outportb(LPTREG_DATA, 0);

        if(!(inportb(LPTREG_STATUS)&STATUS_BUSY))
            read |= (1<<i);
        outportb(LPTREG_DATA, D_CLOCK_HIGH);
    }
    // delay between bytes
    lptdelay(64);
    return read;
}

void gb_sendwrite(U16 addr, U8 val)
{
    gb_sendbyte(0x49);
    gb_sendbyte(addr>>8);
    gb_sendbyte(addr&0xFF);
    gb_sendbyte(val);
}

void gb_sendblockread(U16 addr, U16 length)
{
    gb_sendbyte(0x59);
    gb_sendbyte(addr>>8);
    gb_sendbyte(addr&0xFF);
    gb_sendbyte(length>>8);
    gb_sendbyte(length&0xFF);
}

void GBLINK_CALL ReadBlock(U8 *dest, U16 addr, int len)
{
    logMessage(msg);
    gb_sendblockread(addr,len);
    for(int i=0;i<len;i++)
        dest[i] = gb_readbyte();
}

U8 GBLINK_CALL ReadByte(U16 addr)
{
    gb_sendblockread(addr,1);
    return gb_readbyte();
}

void GBLINK_CALL WriteByte(U16 addr, U8 val)
{
    gb_sendwrite(addr, val);
}

void readBankZero()
{
    // read the first bank of ROM
    logMessage("Downloading first bank...");
    for(int i=0;i<0x4000;i++) {
        bank0[i] = gb_readbyte();
    }
}

bool GBLINK_CALL InitLinker()
{
    if (linkerInitialising) {
        logMessage("Already initialising");
        return false;
    }

    linkerInitialising = true;

    hInpOutDll = LoadLibrary("inpout32.dll");
    if (hInpOutDll != NULL) {
        gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "Out32");
        gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "Inp32");
    } else {
        logMessage("Unable to load inpout32.dll");
        return false;
    }

    logMessage("LibGBlink");
    logMessage("Based on original GBlinkdl by Brian Provinciano & GBlinkDX by taizou");

    logMessage("Setting up ports...");

    // set up the parallel port
    outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
    outportb(LPTREG_DATA, 0xFF);
    outportb(LPTREG_DATA, D_CLOCK_HIGH);

    // perform communication
    logMessage("Waiting for Game Boy...");
    while(gb_sendbyte(0x9A)!=0xB4) {}
    lptdelay(2000);
    if(gb_sendbyte(0x9A)!=0x1D) {
        logMessage("Bad connection");
        return false;
    }

    logMessage("Connected.");

    U8 carttype = gb_readbyte();
    U8 romsize = gb_readbyte();
    U8 ramsize = gb_readbyte();
    U16 checksum = (gb_readbyte()<<8) | (gb_readbyte());

    char gamename[17];
    for(int i=0;i<16;i++)
        gamename[i] = gb_readbyte();
    gamename[16] = '\0';

    sprintf(msg,
            "Cartridge header: Title %s / Cart type %02X / ROM size %02X / RAM size %02X / Checksum %04X",
            gamename,carttype,romsize,ramsize,checksum
    );
    logMessage(msg);

    if(gb_readbyte() != 0) {// verify we're done
        logMessage("expected 0x00 from GB, bad connection");
        return false;
    }
    if(gb_readbyte() != 0xFF) {// verify we're done
        logMessage("expected 0xFF from GB, bad connection");
        return false;
    }

    logMessage("Reading first bank...");

    readBankZero();

    logMessage("Ready!");

    linkerInitialising = false;
    linkerActive = true;

    return true;
}

void GBLINK_CALL DeinitLinker() {
    if (linkerInitialising) {
        return;
    }
    linkerActive = false;
    outportb(LPTREG_DATA, D_CLOCK_HIGH);
    outportb(LPTREG_CONTROL, inportb(LPTREG_CONTROL)&(~CTL_MODE_DATAIN));
    outportb(LPTREG_DATA, 0xFF);
    FreeLibrary(hInpOutDll);
    logMessage("Disconnected");
}

void GBLINK_CALL SetLogger(LinkerLogger newlogger) {
    logger = newlogger;
    logMessage("Setup logger");
}

bool GBLINK_CALL IsLinkerActive() {
    return linkerActive;
}

U8* GBLINK_CALL GetBank0() {
    return bank0;
}
