#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#define BUFFERSIZE 128
#define ESCAPE_RET 27
#define ENTER_RET 0

#define INVALID -1
#define REGISTER 1
#define SETPASS 2
#define SETNAME 3
#define LOGIN 4
#define EXIT 5
#define INVITE 6
#define JOIN 7
#define GETUSERS 8
#define GETALLUSERS 9
#define GETUSER 10
#define LEAVE 11
#define GETMOTD 12
#define GETROOMS 13

#include <time.h>

constexpr int fSize = 8 * 1000000;
constexpr char* escape = "x52nbs";
constexpr int escapelen = 7;

struct Packet {
   time_t timestamp;
   char buf[BUFFERSIZE];
   char username[64];
   char realname[64];
   int options;
};

enum class MessageType : char {
    ServerAccept = 'A',
    ServerDeny = 'D',
    ServerPing = 'P',
    ServerMessage = 'M',
    ClientMessage = 'C',
    ClientMessageUpdate = 'U',
    UserUpdate = 'B',
    UserRequest = 'R',
    ClientDisconnecting = 'E',
    FileSend = 'S'
};

inline std::array<std::string, 14> picExtensions = {
    ".bmp",
    ".gif",
    ".jpeg",
    ".lbm",
    ".pcx",
    ".png",
    ".pnm",
    ".svg",
    ".tga",
    ".tiff",
    ".webp",
    ".xcf",
    ".xpm",
    ".xv"
};

typedef struct Packet packet;