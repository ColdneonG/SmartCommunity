#include "parkingapply.h"
#include "ui_parkingapply.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include "databasemanager.h"

ParkingApply::ParkingApply(int ownerid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingApply),
    m_ownerid(ownerid),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingApply_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }
}

ParkingApply::~ParkingApply()
{
    dbManager->closeDatabase();
    delete ui;
}

void ParkingApply::on_pushButton_apply_clicked()
{
    QString plate = ui->le_plate->text().trimmed();

    QRegExp rx("^[\u4e00-\u9fa5][A-HJ-NP-Z][A-HJ-NP-Z0-9]{4,5}$");
    if (plate.isEmpty() || !rx.exactMatch(plate)) {
        QMessageBox::critical(this, "错误", "车牌号格式不正确！");
        return;
    }

    QString type = ui->comboBox_type->currentText();
    if (type.isEmpty()) {
        QMessageBox::critical(this, "错误", "类型信息不完整！");
        return;
    }

    QString isChargable = ui->comboBox_is_chargeable->currentText();
    if (isChargable.isEmpty()) {
        QMessageBox::critical(this, "错误", "是否需要充电功能信息不完整！");
        return;
    }

    QString size = ui->comboBox_size->currentText();
    if (size.isEmpty()) {
        QMessageBox::critical(this, "错误", "型号信息不完整！");
        return;
    }
    QRegExp rx1("[0-9]+$");
    QString length = ui->le_length->text().trimmed();
    if (length.isEmpty() || !rx1.exactMatch(length)) {
        QMessageBox::critical(this, "错误", "租用时长必须为整数！");
        return;
    }

    QString sql = QString("select count(*) from parking_apply where owner_id=%1 and status!=3").arg(m_ownerid);
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "查询申请记录数量失败");
        return;
    }
    int count = results.first().value(0).toInt();
    if (count >= 1) {
        QMessageBox::information(this, "", "您已经提交过申请了，不能重复提交，请前往“我的车位”查看申请进度");
        return;
    }

    int chargeableValue = (isChargable == "是") ? 1 : 0;
    int rentalLength = length.toInt();

    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yyyy-MM-dd HH:mm:ss");

    sql = QString("insert into parking_apply (owner_id, plate, type, is_chargeable, size, apply_time, length) "
                  "values (%1, '%2', '%3', %4, '%5', '%6', %7)")
          .arg(m_ownerid)
          .arg(plate)
          .arg(type)
          .arg(chargeableValue)
          .arg(size)
          .arg(timeStr)
          .arg(rentalLength);

    if (dbManager->executeQuery(sql)) {
        QMessageBox::information(this, "成功", "车位申请提交成功！请耐心等待。");
        this->close();
    } else {
        QMessageBox::critical(this, "失败", "申请提交失败，请稍后重试！");
    }
}
