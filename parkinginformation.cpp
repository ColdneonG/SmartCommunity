#include "parkinginformation.h"
#include "ui_parkinginformation.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include "databasemanager.h"

ParkingInformation::ParkingInformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingInformation)
{
    QString connName = QString("LeaveApprove_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);

    // 创建唯一的数据库管理器实例
    DatabaseManager* dbManager = new DatabaseManager(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 将同一个dbManager实例传递给两个组件
    widgetAdd = new ParkingAdd(dbManager, this);
    widgetShow = new ParkingShow(dbManager, this);

    mainLayout->addWidget(widgetShow);
    mainLayout->addWidget(widgetAdd);

    widgetShow->show();
    widgetAdd->hide();

    connect(widgetShow, &ParkingShow::addButtonClicked,
            this, &ParkingInformation::showAddWidget);
    connect(widgetAdd, &ParkingAdd::addButtonClicked,
            this, &ParkingInformation::showShowWidget);
}

ParkingInformation::~ParkingInformation()
{
    delete ui;
}

void ParkingInformation::showAddWidget()
{
    widgetShow->hide();
    widgetAdd->show();
}

void ParkingInformation::showShowWidget()
{
    widgetShow->show();
    widgetAdd->hide();
    widgetShow->refreshParkingData();
}
