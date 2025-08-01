﻿#include "../transfer/transferringwidget.h"
#include "zipfileprocesswidget.h"
#include "zipfileprocessresultwidget.h"
#include "../type_defines.h"
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QTextBrowser>
#include <QMovie>
#include <QStackedWidget>

#include <gui/connect/choosewidget.h>
#include <net/helper/transferhepler.h>
#include <utils/optionsmanager.h>
#include <common/commonutils.h>

ZipFileProcessWidget::ZipFileProcessWidget(QWidget *parent) : QFrame(parent)
{
    DLOG << "Widget constructor called";
    initUI();
}

ZipFileProcessWidget::~ZipFileProcessWidget()
{
    DLOG << "Widget destructor called";
}

void ZipFileProcessWidget::updateProcess(const QString &content, int processbar, int estimatedtime)
{
    if (OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0]
        == TransferMethod::kNetworkTransmission) {
        DLOG << "Transfer method is NetworkTransmission, skipping updateProcess";
        return;
    }
    // Transfer success or failure to go to the next page
    if (processbar == 100 || processbar == -1) {
        DLOG << "Zip process completed with status:" << processbar << ", proceeding to next page";
        nextPage();
        return;
    }
    DLOG << "Updating zip process:" << processbar << "% - Current file:" << content.toStdString();
    changeFileLabel(deepin_cross::CommonUitls::elidedText(content, Qt::ElideMiddle, 40));
    changeTimeLabel(estimatedtime);
    changeProgressBarLabel(processbar);
}

void ZipFileProcessWidget::changeFileLabel(const QString &path)
{
    QString info = QString(QString("<font color='#526A7F'>&nbsp;&nbsp;&nbsp;%1</font>")
                                   .arg(tr("Packing  %1").arg(path)));
    DLOG << "Updating current file label to:" << path.toStdString();
    fileLabel->setText(info);
}

void ZipFileProcessWidget::changeTimeLabel(const int &time)
{
    DLOG << "Updating estimated time:" << time << "seconds";
    if (time > 60) {
        int textTime = time / 60;
        timeLabel->setText(QString(
                tr("Transfer will be completed in %1 minutes").arg(QString::number(textTime))));
        DLOG << "Converted to minutes:" << textTime;
    } else {
        DLOG << "Time is 60 seconds or less, displaying in seconds";
        timeLabel->setText(QString(
                tr("Transfer will be completed in %1 secondes").arg(QString::number(time))));
    }
}

void ZipFileProcessWidget::changeProgressBarLabel(const int &processbar)
{
    DLOG << "Updating progress bar to:" << processbar << "%";
    progressLabel->setProgress(processbar);
}

void ZipFileProcessWidget::initUI()
{
    DLOG << "ZipFileProcessWidget initUI";
    setStyleSheet(".ZipFileProcessWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(30);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QMovie *iconmovie = new QMovie(this);
    iconmovie->setFileName(":/icon/GIF/light/compress.gif");
    iconmovie->setScaledSize(QSize(200, 160));
    iconmovie->setSpeed(80);
    iconmovie->start();
    iconLabel->setMovie(iconmovie);

    QLabel *titileLabel = new QLabel(tr("Creating Backup File..."), this);
   titileLabel->setFixedHeight(50);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignHCenter);

    progressLabel = new ProgressBarLabel(this);
    progressLabel->setProgress(0);
    progressLabel->setFixedSize(280, 8);

    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(progressLabel, Qt::AlignCenter);

    timeLabel = new QLabel(this);
    timeLabel->setText(
            QString(tr("Transfer will be completed in %1 minutes").arg(QString::number(0))));
    timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    fileLabel = new QLabel(this);
    fileLabel->setFixedHeight(15);
    fileLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    IndexLabel *indelabel = new IndexLabel(3, this);
    indelabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    mainLayout->addSpacing(50);
    mainLayout->addWidget(iconLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(progressLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(timeLabel);
    mainLayout->addWidget(fileLabel);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(indexLayout);

    QObject::connect(TransferHelper::instance(), &TransferHelper::zipTransferContent, this,
                     &ZipFileProcessWidget::updateProcess);
    DLOG << "ZipFileProcessWidget initUI finished";
}

void ZipFileProcessWidget::nextPage()
{
    DLOG << "Navigation initiated to result widget";
    emit TransferHelper::instance()->changeWidget(PageName::zipfileprocessresultwidget);
}
