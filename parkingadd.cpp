#include "parkingadd.h"
#include "ui_parkingadd.h"
#include <QMessageBox>

ParkingAdd::ParkingAdd(DatabaseManager* dbManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingAdd),
    dbManager(dbManager)
{
    ui->setupUi(this);
    connect(ui->pushButton_exit, &QPushButton::clicked, this, &ParkingAdd::addButtonClicked);
}

ParkingAdd::~ParkingAdd()
{
    delete ui;
}

void ParkingAdd::on_pushButton_ok_clicked()
{
    QString location = ui->le_location->text().trimmed();
    if (location.isEmpty()) {
        QMessageBox::critical(this, "错误", "车位编号信息不完整！");
        return;
    }

    QString type = ui->comboBox_type->currentText();
    if (type.isEmpty()) {
        QMessageBox::critical(this, "错误", "类型信息不完整！");
        return;
    }

    QString isChargable = ui->comboBox_is_chargeable->currentText();
    if (isChargable.isEmpty()) {
        QMessageBox::critical(this, "错误", "是否可充电信息不完整！");
        return;
    }

    QString size = ui->comboBox_size->currentText();
    if (size.isEmpty()) {
        QMessageBox::critical(this, "错误", "型号信息不完整！");
        return;
    }

    int chargableValue = (isChargable == "是") ? 1 : 0;

    // 构建SQL插入语句
    QString sql = "INSERT INTO parking_space (location, type, is_chargeable, size, status) "
                  "VALUES (:location, :type, :is_chargeable, :size, 0)";
    QVariantList bindValues;
    bindValues << location << type << chargableValue << size;

    if (dbManager->executeQuery(sql, bindValues)) {
        QMessageBox::information(this, "成功", "车位信息添加成功！");
        ui->le_location->clear();
        ui->comboBox_type->setCurrentIndex(0);
        ui->comboBox_is_chargeable->setCurrentIndex(0);
        ui->comboBox_size->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "错误", "添加车位失败：" + dbManager->lastError());
    }
}

void ParkingAdd::on_pushButton_exit_clicked()
{
    emit addButtonClicked();
}
