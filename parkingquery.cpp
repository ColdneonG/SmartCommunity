#include "parkingquery.h"
#include "ui_parkingquery.h"
#include <QMessageBox>
#include <QDateTime>
#include "databasemanager.h"

ParkingQuery::ParkingQuery(int ownerId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingQuery),
    ownerId(ownerId),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingQuery_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }

    init();
}

ParkingQuery::~ParkingQuery()
{
    dbManager->closeDatabase();
    delete ui;
}

void ParkingQuery::init(){
    QString sql = QString("select * from parking_apply where owner_id=%1 and status!=3 ").arg(ownerId);
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        applyLabel = new QLabel("您还没有申请车位或车位已过期，请前往“车位申请”。", this);
        applyLabel->setGeometry(0, 300, 1360, 100);
        applyLabel->setFont(QFont("SimHei", 14));
        applyLabel->setAlignment(Qt::AlignCenter);
        return;
    }
    QVariantMap result = results.first();
    QString plate = result["plate"].toString();
    int status = result["status"].toInt();
    if (status == 0) {
        applyLabel = new QLabel("工作人员还没有处理您的申请，请耐心等待”。", this);
        applyLabel->setGeometry(0, 300, 1360, 100);
        applyLabel->setFont(QFont("SimHei", 14));
        applyLabel->setAlignment(Qt::AlignCenter);
        return;
    }

    sql = QString("select * from parking_rental where owner_id=%1 and payment_status!=3").arg(ownerId);
    results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到租赁记录");
        return;
    }
    result = results.first();
    int spaceId = result["space_id"].toInt();
    QString endTime = result["end_time"].toString();
    QString fee = result["rent_fee"].toString();
    QString paymentStatus = result["payment_status"].toString();

    sql = QString("select * from parking_space where space_id =%1").arg(spaceId);
    results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到车位记录");
        return;
    }
    result = results.first();
    QString spaceName = result["location"].toString();

    QString displayText = plate + "车主，您好：\n"
                          "车位编号: " + spaceName + "\n"
                          "结束时间: " + endTime + "\n";

    if (paymentStatus == "1") {
        displayText += "缴费状态: 已缴费";
    } else {
        displayText += "缴费状态: 未缴费  待缴费金额: " + fee + "元";
    }

    applyLabel = new QLabel(displayText, this);
    applyLabel->setGeometry(50, 200, 1260, 200);
    applyLabel->setFont(QFont("SimHei", 14));
    applyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    applyLabel->setWordWrap(true);
}
