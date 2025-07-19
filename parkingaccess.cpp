#include "parkingaccess.h"
#include "ui_parkingaccess.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include "databasemanager.h"

ParkingAccess::ParkingAccess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingAccess),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingAccess_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }
}

ParkingAccess::~ParkingAccess()
{
    dbManager->closeDatabase();
    delete ui;
}

void ParkingAccess::on_pushButton_ok_clicked()
{
    QString plate = ui->le_plate->text().trimmed();
    QString type = ui->comboBox_type->currentText();
    QRegExp rx("^[\u4e00-\u9fa5][A-HJ-NP-Z][A-HJ-NP-Z0-9]{4,5}$");
    if (plate.isEmpty() || !rx.exactMatch(plate)) {
        QMessageBox::critical(this, "错误", "车牌号不能正确识别！");
        return;
    }

    QString sql = QString("select * from parking_rental where plate='%1' and payment_status!=3").arg(plate);
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "", "该车辆没有车位使用权！");
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    int in = (type == "进入") ? 1 : 0;
    sql = QString("insert into vehicle_access_log (plate,type,time)"
                  "values('%1',%2,'%3')")
          .arg(plate)
          .arg(in)
          .arg(currentTime.toString("yyyy-MM-dd hh:mm:ss"));
    if (!dbManager->executeQuery(sql)) {
        QMessageBox::critical(this, "错误", "插入车辆进出记录失败: " + dbManager->lastError());
        return;
    }

    sql = QString("select * from parking_rental where plate='%1'").arg(plate);
    results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到租赁记录");
        return;
    }
    QVariantMap result = results.first();
    QString endTime = result["end_time"].toString();
    QMessageBox::information(this, "提醒", QString("车位到期时间%1").arg(endTime));
}
