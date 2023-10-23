// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMSHARE_H
#define COMSHARE_H

#include "message.pb.h"
#include <co/co.h>
#include <co/json.h>

typedef enum income_type_t {
    IN_LOGIN_RESULT= 100,
    IN_LOGIN_CONFIRM = 1000,
    IN_TRANSJOB = 1001,
    IN_PEER = 1002,
    FS_ACTION = 1003,
    FS_DATA = 1004,
    FS_DONE = 1005,
    FS_INFO = 1006,
    FS_REPORT = 1007,
    TRANS_CANCEL = 1008,
} IncomeType;

typedef enum outgo_type_t {
    OUT_LOGIN = 2000,
    OUT_TRANSJOB = 2001,
    OUT_PEER = 2002,
    FS_ACTION_RESULT = 2003,
} OutgoType;

struct IncomeData {
    IncomeType type;
    fastring json; // json数据结构实例
    fastring buf; // 二进制数据
};

struct OutData {
    OutgoType type;
    fastring json; // json数据结构实例
};

extern co::chan<IncomeData> _income_chan;
extern co::chan<OutData> _outgo_chan;


class comshare
{
public:
    comshare();
};

#endif // COMSHARE_H