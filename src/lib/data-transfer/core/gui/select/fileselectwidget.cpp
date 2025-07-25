﻿#include "siderbarwidget.h"
#include "fileselectwidget.h"

#include "calculatefilesize.h"
#include "userselectfilesize.h"
#include "../type_defines.h"
#include "common/log.h"

#include <utils/optionsmanager.h>
#include <net/helper/transferhepler.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QDir>

FileSelectWidget::FileSelectWidget(SidebarWidget *siderbarWidget, QWidget *parent)
    : QFrame(parent), sidebar(siderbarWidget)
{
    DLOG << "Initializing file selection widget";

    initUI();

    QObject::connect(sidebar, &SidebarWidget::updateFileview, this,
                     &FileSelectWidget::updateFileViewData);

    // update fileview file size
    QObject::connect(CalculateFileSizeThreadPool::instance(),
                     &CalculateFileSizeThreadPool::sendFileSizeSignal, this,
                     &FileSelectWidget::updateFileViewSize);

    QObject::connect(sidebar, &QListView::clicked, this, &FileSelectWidget::changeFileView);
    DLOG << "File selection widget initialized";
}

FileSelectWidget::~FileSelectWidget()
{
    DLOG << "Destroying file selection widget";
}

void FileSelectWidget::initUI()
{
    DLOG << "Initializing UI components";

    setStyleSheet("background-color: white; border-radius: 10px;");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    titileLabel = new QLabel(LocalText, this);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    titlebar = new ItemTitlebar(tr("Name"), tr("Size"), 50, 360, QRectF(10, 8, 16, 16), 3, this);
    titlebar->setFixedSize(500, 36);
    headerLayout->addWidget(titlebar);

    QHBoxLayout *fileviewLayout = new QHBoxLayout();
    stackedWidget = new QStackedWidget(this);
    initFileView();
    fileviewLayout->addWidget(stackedWidget);

    QLabel *tipLabel1 = new QLabel(
            tr("When transfer completed, the data will be placed in the user's home directory"),
            this);
    tipLabel1->setWordWrap(true);
    tipLabel1->setAlignment( Qt::AlignHCenter);
    tipLabel1->setFixedHeight(30);
    font.setPointSize(10);
    font.setWeight(QFont::Thin);
    tipLabel1->setFont(font);

    ButtonLayout *buttonLayout = new ButtonLayout();
    QPushButton *cancelButton = buttonLayout->getButton1();
    cancelButton->setText(tr("Cancel"));
    QPushButton * determineButton = buttonLayout->getButton2();
    determineButton->setText(tr("Confirm"));

    connect(determineButton, &QToolButton::clicked, this, &FileSelectWidget::nextPage);
    connect(cancelButton, &QToolButton::clicked, this, &FileSelectWidget::backPage);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(tipLabel1);
    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(fileviewLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    QObject::connect(titlebar, &ItemTitlebar::selectAll, this,
                     &FileSelectWidget::selectOrDelAllItem);
    QObject::connect(sidebar, &SidebarWidget::selectOrDelAllFileViewItem, this,
                     &FileSelectWidget::selectOrDelAllItemFromSiderbar);
    QObject::connect(sidebar, &SidebarWidget::updateSelectBtnState, this,
                     &FileSelectWidget::updateTitleSelectBtnState);

    auto sortBtn = titlebar->getSortButton1();
    sortBtn->setVisible(true);
    titlebar->setType(false);
    QObject::connect(sortBtn, &SortButton::sort, this, &FileSelectWidget::sortListview);

    auto sortBtn2 = titlebar->getSortButton2();
    sortBtn2->setVisible(true);
    QObject::connect(sortBtn2, &SortButton::sort, this, &FileSelectWidget::sortListviewColumn2);
    DLOG << "File selection widget initialized";
}

void FileSelectWidget::startCalcluateFileSize(QList<QString> fileList)
{
    DLOG << "Starting file size calculation for" << fileList.size() << "files";

    CalculateFileSizeThreadPool::instance()->work(fileList);
}

void FileSelectWidget::initFileView()
{
    DLOG << "Initializing file views";

    QMap<QStandardItem *, DiskInfo> *diskList = sidebar->getSidebarDiskList();
    for (auto iterator = diskList->begin(); iterator != diskList->end(); ++iterator) {
        QStandardItem *diskItem = iterator.key();
        DiskInfo diskInfo = iterator.value();
        QString path = diskInfo.rootPath;
        SelectListView *view = addFileViewData(path, diskItem);
        sidebarFileViewList[diskItem] = view;
        stackedWidget->addWidget(view);
        // DLOG << "Added file view for disk:" << path.toStdString();
    }
    // init sidebar user directory size
    sidebar->initSiderDataAndUi();

    stackedWidget->setCurrentIndex(0);
    DLOG << "File views initialized";
}

void FileSelectWidget::changeFileView(const QModelIndex &siderbarIndex)
{
    DLOG << "Changing file view for sidebar item";
    QString path = sidebar->model()->data(siderbarIndex, Qt::UserRole).toString();
    QMap<QStandardItem *, DiskInfo> *list = sidebar->getSidebarDiskList();
    for (auto iteraotr = list->begin(); iteraotr != list->end(); ++iteraotr) {
        QString rootPath = iteraotr.value().rootPath;
        if (path == rootPath) {
            DLOG << "Matching root path found:" << rootPath.toStdString();
            QStandardItem *item = iteraotr.key();
            stackedWidget->setCurrentWidget(sidebarFileViewList[item]);
            auto state = static_cast<ListSelectionState>(item->data(Qt::StatusTipRole).toInt());
            titlebar->updateSelectAllButState(state);
        } else {
            DLOG << "No matching root path for:" << path.toStdString();
        }
    }
}

void FileSelectWidget::selectOrDelAllItem()
{
    DLOG << "Toggling select/deselect all items";

    SelectListView *listview = qobject_cast<SelectListView *>(stackedWidget->currentWidget());
    listview->selectorDelAllItem();
}

void FileSelectWidget::updateFileViewSize(quint64 fileSize, const QString &path)
{
    DLOG << "Updating file size for path:" << path.toStdString();
    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    QStandardItem *item = (*filemap)[path].fileItem;
    item->setData(fromByteToQstring(fileSize), Qt::ToolTipRole);
}

void FileSelectWidget::updateFileViewData(QStandardItem *siderbarItem, const bool &isAdd)
{
    DLOG << "Updating file view data for sidebar item";
    // del fileview
    if (!isAdd) {
        DLOG << "Removing file view for sidebar item";
        SelectListView *view = qobject_cast<SelectListView *>(sidebarFileViewList[siderbarItem]);
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(view);
        sidebarFileViewList.remove(siderbarItem);
        delete view;
        LOG << "del device";

    } else {
        // add fileview
        DLOG << "Adding file view for sidebar item";
        QMap<QStandardItem *, DiskInfo> *diskList = sidebar->getSidebarDiskList();
        QString path = diskList->value(siderbarItem).rootPath;
        LOG << "updateFileViewData add " << path.toStdString();
        SelectListView *view = addFileViewData(path, siderbarItem);
        sidebarFileViewList[siderbarItem] = view;
        stackedWidget->addWidget(view);
        LOG << "add device";
    }
    DLOG << "File view data updated for sidebar item";
}

void FileSelectWidget::selectOrDelAllItemFromSiderbar(QStandardItem *siderbarItem)
{
    DLOG << "Toggling select/deselect all items for sidebar item";
    SelectListView *listview =
            qobject_cast<SelectListView *>(sidebarFileViewList.value(siderbarItem));
    listview->selectorDelAllItem();
}

void FileSelectWidget::updateTitleSelectBtnState(QStandardItem *siderbarItem,
                                                 ListSelectionState state)
{
    DLOG << "Updating titlebar select button state for sidebar item";
    SelectListView *listview =
            qobject_cast<SelectListView *>(sidebarFileViewList.value(siderbarItem));
    if (sidebarFileViewList[siderbarItem] != listview)
        return;
    titlebar->updateSelectAllButState(state);
}

void FileSelectWidget::sortListviewColumn2()
{
    DLOG << "Sorting by column 2 (size)";

    for (auto iteraotr = sidebarFileViewList.begin(); iteraotr != sidebarFileViewList.end();
         ++iteraotr) {
        auto view = qobject_cast<SelectListView *>(iteraotr.value());
        view->sortListview();
        view->setSortRole(false);
    }
}

void FileSelectWidget::sortListview()
{
    DLOG << "Sorting by column 1 (name)";

    for (auto iteraotr = sidebarFileViewList.begin(); iteraotr != sidebarFileViewList.end();
         ++iteraotr) {
        auto view = qobject_cast<SelectListView *>(iteraotr.value());
        view->sortListview();
        view->setSortRole(true);
    }
}

SelectListView *FileSelectWidget::addFileViewData(const QString &path, QStandardItem *sidebaritem)
{
    DLOG << "Adding file view data for path:" << path.toStdString();
    QDir directory(path);
    // del . and ..
    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList fileinfos = directory.entryInfoList();

    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    ItemDelegate *delegate = new ItemDelegate(99, 250, 379, 100, 50, QPoint(65, 6), QPoint(14, 9));

    SelectListView *fileView = new SelectListView(this);
    fileView->setSortRole(false);
    QStandardItemModel *model = fileView->getModel();
    fileView->setItemDelegate(delegate);

    int diskFileNUM = 0;
    QList<QString> needCalculateFileList;
    for (int i = 0; i < fileinfos.count(); i++) {
        if (!fileinfos[i].isDir()) {
            // DLOG << "File info is not a directory, skipping:" << fileinfos[i].fileName().toStdString();
            continue;
        }
        if (fileinfos[i].fileName() == "Users") {
            // DLOG << "File name is 'Users', skipping:" << fileinfos[i].fileName().toStdString();
            continue;
        }
        QStandardItem *item = new QStandardItem();
        item->setData(fileinfos[i].fileName(), Qt::DisplayRole);
        item->setData("", Qt::ToolTipRole);
        item->setData(fileinfos[i].filePath(), Qt::UserRole);
        item->setIcon(QIcon(":/icon/folder.svg"));
        item->setCheckable(true);
        item->setData(1, Qt::ToolTipPropertyRole);
        model->appendRow(item);

        // add fileInfo
        FileInfo fileInfo;
        fileInfo.size = 0;
        fileInfo.isCalculate = false;
        fileInfo.isSelect = false;
        fileInfo.fileItem = item;
        fileInfo.siderbarItem = sidebaritem;
        (*filemap)[fileinfos[i].filePath()] = fileInfo;

        needCalculateFileList.push_back(fileinfos[i].filePath());

        ++diskFileNUM;
    }
    for (int i = 0; i < fileinfos.count(); i++) {
        if (!fileinfos[i].isFile() || fileinfos[i].isSymLink()) {
            // DLOG << "File info is not a file or is a symlink, skipping:" << fileinfos[i].fileName().toStdString();
            continue;
        }
        QStandardItem *item = new QStandardItem();
        item->setData(fileinfos[i].fileName(), Qt::DisplayRole);
        item->setData(fromByteToQstring(fileinfos[i].size()), Qt::ToolTipRole);
        item->setIcon(QIcon(":/icon/file@2x.png"));
        item->setData(fileinfos[i].filePath(), Qt::UserRole);
        item->setData(2, Qt::ToolTipPropertyRole);
        item->setCheckable(true);
        model->appendRow(item);

        // add fileInfo
        FileInfo fileInfo;
        fileInfo.size = fileinfos[i].size();
        fileInfo.isCalculate = true;
        fileInfo.isSelect = false;
        fileInfo.fileItem = item;
        fileInfo.siderbarItem = sidebaritem;
        (*filemap)[fileinfos[i].filePath()] = fileInfo;

        ++diskFileNUM;
    }

    // add disk file num
    sidebar->addDiskFileNum(sidebaritem, diskFileNUM);

    // calculate filelist
    startCalcluateFileSize(needCalculateFileList);

    QObject::connect(model, &QStandardItemModel::itemChanged, UserSelectFileSize::instance(),
                     &UserSelectFileSize::updateFileSelectList);

    DLOG << "File view data added for path:" << path.toStdString();
    return fileView;
}

void FileSelectWidget::changeText()
{
    DLOG << "Updating title text";
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
}

void FileSelectWidget::clear()
{
    DLOG << "Clearing file select widget";
    int pageCount = stackedWidget->count();
    for (int i = 0; i < pageCount; ++i) {
        SelectListView *currentPage = qobject_cast<SelectListView *>(stackedWidget->widget(i));
        QStandardItemModel *model = currentPage->getModel();
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex itemIndex = model->index(row, 0);
            model->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    OptionsManager::instance()->addUserOption(Options::kFile, QStringList());
    DLOG << "File select widget cleared";
}

void FileSelectWidget::updateFileSelectList(QStandardItem *item)
{
    DLOG << "Updating file select list";
    QString path = item->data(Qt::UserRole).toString();
    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    if (item->data(Qt::CheckStateRole) == Qt::Unchecked) {
        if ((*filemap)[path].isSelect == false) {
            DLOG << "File is already deselected, returning";
            return;
        }
        // do not select the file
        (*filemap)[path].isSelect = false;
        UserSelectFileSize::instance()->delSelectFiles(path);
        if ((*filemap)[path].isCalculate) {
            DLOG << "File size is calculated, deleting user select file size";
            quint64 size = (*filemap)[path].size;
            UserSelectFileSize::instance()->delUserSelectFileSize(size);
        } else {
            DLOG << "File size is not calculated, deleting pending files";
            UserSelectFileSize::instance()->delPendingFiles(path);
        }
    } else if (item->data(Qt::CheckStateRole) == Qt::Checked) {
        if ((*filemap)[path].isSelect == true) {
            DLOG << "File is already selected, returning";
            return;
        }
        (*filemap)[path].isSelect = true;
        UserSelectFileSize::instance()->addSelectFiles(path);
        if ((*filemap)[path].isCalculate) {
            DLOG << "File size is calculated, adding user select file size";
            quint64 size = (*filemap)[path].size;
            UserSelectFileSize::instance()->addUserSelectFileSize(size);
        } else {
            DLOG << "File size is not calculated, adding pending files";
            UserSelectFileSize::instance()->addPendingFiles(path);
        }
    } else {
        DLOG << "Unknown check state for item:" << item->data(Qt::CheckStateRole).toInt();
    }
    DLOG << "File select list updated for path:" << path.toStdString();
}

void FileSelectWidget::nextPage()
{
    DLOG << "Navigating to next page";

    // send useroptions
    sendOptions();

    // nextpage
    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}

void FileSelectWidget::backPage()
{
    DLOG << "Returning to previous page";

    // delete Options
    delOptions();

    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}

void FileSelectWidget::sendOptions()
{
    DLOG << "Sending selected file options";

    QStringList selectFileLsit = UserSelectFileSize::instance()->getSelectFilesList();
    OptionsManager::instance()->addUserOption(Options::kFile, selectFileLsit);
    qInfo() << "select file:" << selectFileLsit;

    emit isOk(SelectItemName::FILES);
}

void FileSelectWidget::delOptions()
{
    DLOG << "Clearing selected file options";

    // Clear All File Selections
    QStringList filelist = UserSelectFileSize::instance()->getSelectFilesList();
    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    for (QString filePath : filelist) {
        QStandardItem *item = (*filemap)[filePath].fileItem;
        item->setCheckState(Qt::Unchecked);
    }
    OptionsManager::instance()->addUserOption(Options::kFile, QStringList());
    emit isOk(SelectItemName::FILES);
}
