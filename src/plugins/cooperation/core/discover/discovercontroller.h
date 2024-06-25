﻿// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCOVERYCONTROLLER_H
#define DISCOVERYCONTROLLER_H

#include "global_defines.h"
#include "info/deviceinfo.h"
#include <QObject>
#include "qzeroconf.h"

namespace cooperation_core {
class DiscoverControllerPrivate;
class DiscoverController : public QObject
{
    Q_OBJECT
public:
    static DiscoverController *instance();

    void publish();
    void unpublish();
    void updatePublish();
    void refresh();
    void startDiscover();

    QList<DeviceInfoPointer> getOnlineDeviceList() const;
    DeviceInfoPointer findDeviceByIP(const QString &ip);
    static DeviceInfoPointer selfInfo();

    void updateDeviceState(const DeviceInfoPointer info);

    static bool isZeroConfDaemonActive();
    static bool openZeroConfDaemonDailog();

Q_SIGNALS:
    void deviceOnline(const QList<DeviceInfoPointer> &infoList);
    void deviceOffline(const QString &ip);
    void startDiscoveryDevice();
    void discoveryFinished(bool hasFound);

private Q_SLOTS:
    void addService(QZeroConfService zcs);
    void removeService(QZeroConfService zcs);
    void updateService(QZeroConfService zcs);

    void onDConfigValueChanged(const QString &config, const QString &key);
    void onAppAttributeChanged(const QString &group, const QString &key, const QVariant &value);

    void addSearchDeivce(const QString &info);

private:
    explicit DiscoverController(QObject *parent = nullptr);
    ~DiscoverController();

    void initZeroConf();
    void initConnect();
    void initSearchDeivce();
    bool isVaildDevice(const DeviceInfoPointer info);
    DeviceInfoPointer parseDeviceJson(const QString &info);
    DeviceInfoPointer parseDeviceService(QZeroConfService zcs);

private:
    QSharedPointer<DiscoverControllerPrivate> d { nullptr };
};

}   // namespace cooperation_core

#endif   // DISCOVERYCONTROLLER_H