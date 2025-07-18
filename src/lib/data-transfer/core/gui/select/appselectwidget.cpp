﻿#include "appselectwidget.h"
#include "../type_defines.h"
#include "../win/drapwindowsdata.h"
#include "item.h"
#include "common/log.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QCheckBox>
#include <QPainter>
#include <QPainterPath>
#include <QListView>

#include <utils/optionsmanager.h>
#include <net/helper/transferhepler.h>
#include <gui/mainwindow_p.h>

AppSelectWidget::AppSelectWidget(QWidget *parent) : QFrame(parent)
{
    DLOG << "Initializing application selection widget";
    initUI();
}

AppSelectWidget::~AppSelectWidget()
{
    DLOG << "Destroying application selection widget";
}

void AppSelectWidget::initUI()
{
    DLOG << "Initializing UI components";

    setStyleSheet(".AppSelectWidget{background-color: white; border-radius: 8px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    titileLabel = new QLabel(LocalText, this);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);



    QLabel *tipLabel1 = new QLabel(tr("Check transfer application will automatically install the "
                                      "corresponding UOS version of the application."),
                                   this);
    tipLabel1->setWordWrap(true);
    tipLabel1->setFixedHeight(30);
    tipLabel1->setAlignment(Qt::AlignCenter);
    font.setPointSize(10);
    font.setWeight(QFont::Thin);
    tipLabel1->setFont(font);

    ButtonLayout *buttonLayout = new ButtonLayout();
    cancelButton = buttonLayout->getButton1();
    cancelButton->setText(tr("Cancel"));
    determineButton = buttonLayout->getButton2();
    determineButton->setText(tr("Confirm"));

    connect(determineButton, &QPushButton::clicked, this, &AppSelectWidget::nextPage);
    connect(cancelButton, &QPushButton::clicked, this, &AppSelectWidget::backPage);

    initSelectFrame();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(tipLabel1);
    mainLayout->addWidget(selectFrame);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);
    DLOG << "Application selection widget initialized";
}

void AppSelectWidget::initSelectFrame()
{
    DLOG << "Initializing application selection frame";

    QVBoxLayout *selectframeLayout = new QVBoxLayout();
    selectframeLayout->setContentsMargins(1, 1, 1, 1);
    ItemTitlebar *titlebar = new ItemTitlebar(tr("Application"), tr("Recommendation"), 40, 360,
                                              QRectF(10, 8, 16, 16), 3, this);
    titlebar->setFixedSize(500, 36);

    selectFrame = new QFrame(this);
    selectFrame->setContentsMargins(0, 0, 0, 0);
    selectFrame->setFixedSize(500, 318);
    selectFrame->setStyleSheet(".QFrame{"
                               "border-radius: 8px;"
                               "border: 2px solid rgba(0,0,0, 0.1);"
                               "opacity: 1;"
                               "background-color: rgba(255,255,255, 1);"
                               "}");
    selectFrame->setLayout(selectframeLayout);

    appView = new SelectListView(this);
    QStandardItemModel *model = appView->getModel();
    appView->setItemDelegate(new ItemDelegate(84, 250, 376, 100, 40, QPoint(52, 6), QPoint(10, 9)));

    QMap<QString, QString> noRecommendList;
    QMap<QString, QString> appList = TransferHelper::instance()->getAppList(noRecommendList);
    appView->setAllSize(appList.size());
    for (auto iterator = appList.begin(); iterator != appList.end(); ++iterator) {
        QStandardItem *item = new QStandardItem();
        item->setData(iterator.key(), Qt::DisplayRole);
        item->setData(tr("Transferable"), Qt::ToolTipRole);
        item->setData(1, Qt::ToolTipPropertyRole);
        item->setIcon(QIcon(iterator.value()));
        item->setCheckable(true);
        model->appendRow(item);
        DLOG << "Added transferable app:" << iterator.key().toStdString();
    }

    for (auto iterator = noRecommendList.begin(); iterator != noRecommendList.end(); ++iterator) {
        QStandardItem *item = new QStandardItem();
        item->setData(iterator.key(), Qt::DisplayRole);
        item->setData(tr("Not Suitable"), Qt::ToolTipRole);
        item->setData(true, Qt::BackgroundRole);
        item->setData(2, Qt::ToolTipPropertyRole);
        QPixmap pix = DrapWindowsData::instance()->getAppIcon(iterator.value());
        if (pix.isNull()) {
            DLOG << "App icon is null, using default file icon";
            item->setIcon(QIcon(":/icon/file@2x.png"));
        } else {
            DLOG << "App icon found, setting it";
            item->setIcon(QIcon(pix));
        }

        item->setCheckable(false);

        model->appendRow(item);
        DLOG << "Added non-suitable app:" << iterator.key().toStdString();
    }

    selectframeLayout->addWidget(titlebar);
    selectframeLayout->addWidget(appView);

    QObject::connect(titlebar, &ItemTitlebar::selectAll, appView,
                     &SelectListView::selectorDelAllItem);
    QObject::connect(appView, &SelectListView::currentSelectState, titlebar,
                     &ItemTitlebar::updateSelectAllButState);
    auto sortBtn = titlebar->getSortButton1();
    sortBtn->setVisible(true);
    QObject::connect(sortBtn, &SortButton::sort, appView, &SelectListView::sortListview);
    DLOG << "Application selection frame initialized";
}

void AppSelectWidget::changeText()
{
    DLOG << "Updating application selection widget text";
    QString method = OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0];
    if (method == TransferMethod::kLocalExport) {
        DLOG << "Transfer method is LocalExport, setting title to LocalText";
        titileLabel->setText(LocalText);
    } else if (method == TransferMethod::kNetworkTransmission) {
        DLOG << "Transfer method is NetworkTransmission, setting title to InternetText";
        titileLabel->setText(InternetText);
    } else {
        DLOG << "Unknown transfer method:" << method.toStdString();
    }
    DLOG << "Application selection widget text updated";
}

void AppSelectWidget::clear()
{
    DLOG << "Clearing selected applications";
    QStandardItemModel *configmodel = appView->getModel();
    for (int row = 0; row < configmodel->rowCount(); ++row) {
        QModelIndex itemIndex = configmodel->index(row, 0);
        configmodel->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
    }
    OptionsManager::instance()->addUserOption(Options::kApp, QStringList());
    DLOG << "Selected applications cleared";
}

void AppSelectWidget::sendOptions()
{
    DLOG << "Sending selected application options";

    QStringList appName;
    QAbstractItemModel *model = appView->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            appName << model->data(index, Qt::DisplayRole).toString();
        }
    }

    qInfo() << "select app :" << appName;
    OptionsManager::instance()->addUserOption(Options::kApp, appName);

    emit isOk(SelectItemName::APP);
    DLOG << "Selected application options sent";
}

void AppSelectWidget::delOptions()
{
    DLOG << "Clearing selected application options";

    // Clear All App Selections
    QAbstractItemModel *model = appView->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    OptionsManager::instance()->addUserOption(Options::kApp, QStringList());

    // Deselect
    emit isOk(SelectItemName::APP);
    DLOG << "Selected application options cleared";
}

void AppSelectWidget::nextPage()
{
    DLOG << "Navigating to next page";

    // send useroptions
    sendOptions();

    // nextpage
    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}
void AppSelectWidget::backPage()
{
    DLOG << "Returning to previous page";

    // delete Options
    delOptions();
    // backpage
    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}
