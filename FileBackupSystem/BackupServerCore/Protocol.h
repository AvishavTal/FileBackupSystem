#pragma once
#include <cstdint>

// קודי הפעולה
enum MessageOp : uint8_t {
    OP_SAVE_FILE = 200,
    OP_RESTORE_FILE = 201,
    OP_DELETE_FILE = 202,
    OP_LIST_FILES = 203
};

// סטטוסי תשובה
enum MessageStatus : uint16_t {
    STATUS_SUCCESS_FILE_FOUND = 210,
    STATUS_SUCCESS_LIST = 211,
    STATUS_SUCCESS_GENERIC = 212,
    STATUS_ERROR_FILE_NOT_FOUND = 1001,
    STATUS_ERROR_NO_FILES = 1002,
    STATUS_ERROR_GENERAL = 1003
};

// כדי למנוע Padding של הקומפיילר ולהבטיח גודל מדויק
#pragma pack(push, 1)

// המבנה הקבוע בתחילת בקשה (סה"כ 8 בייטים)
struct RequestHeaderFixed {
    uint32_t userId;
    uint8_t version;
    uint8_t op;
    uint16_t nameLen;
};

// המבנה הקבוע בתחילת תשובה (סה"כ 5 בייטים)
struct ResponseHeaderFixed {
    uint8_t version;
    uint16_t status;
    uint16_t nameLen;
};

#pragma pack(pop)