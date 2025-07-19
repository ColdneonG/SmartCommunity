#include "estateregister.h"
#include "ui_estateregister.h"
#include <QDateTime>
#include <QVariant>
#include <QDebug>

EstateRegister::EstateRegister(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EstateRegister),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("EstateRegister_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);

    if (!dbManager->initDatabase(connName)) {
        qDebug() << "数据库初始化失败";
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }
}

EstateRegister::~EstateRegister()
{
    dbManager->closeDatabase();
    delete ui;
}

bool EstateRegister::validateInput()
{
    if (ui->edit_owner_id->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入业主ID");
        return false;
    }

    if (ui->edit_address->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入房产地址");
        return false;
    }

    // 验证数字类型
    bool ownerIdOk;
    ui->edit_owner_id->text().toInt(&ownerIdOk);

    if (!ownerIdOk) {
        QMessageBox::warning(this, "输入错误", "业主ID必须为整数");
        return false;
    }

    return true;
}

bool EstateRegister::executeQuery(const QString &sql, const QVariantList &bindValues)
{
    return dbManager->executeQuery(sql, bindValues);
}

QString EstateRegister::lastError()
{
    return dbManager->lastError();
}

void EstateRegister::on_btn_save_clicked()
{
    if (!dbManager->isOpen()) {
        QMessageBox::critical(this, "数据库错误", "数据库未连接");
        return;
    }

    if (!validateInput()) {
        return;
    }

    // 准备SQL语句 - 使用位置绑定
    QString sql = "INSERT INTO property_info ("
                  "owner_id, address, property_type, "
                  "build_area, property_status, register_time) "
                  "VALUES (?, ?, ?, ?, ?, ?)";

    // 准备绑定值，时间自动获取当前时间
    QVariantList bindValues;
    bindValues << ui->edit_owner_id->text().toInt()
               << ui->edit_address->text()
               << ui->combo_property_type->currentText()
               << ui->spin_build_area->value()
               << ui->combo_property_status->currentText()
               << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    qDebug() << "执行SQL:" << sql;
    qDebug() << "绑定参数:" << bindValues;

    // 执行插入
    if (executeQuery(sql, bindValues)) {
        QMessageBox::information(this, "成功", "房产信息保存成功");
        // 清空输入框
        ui->edit_owner_id->clear();
        ui->edit_address->clear();
        ui->spin_build_area->setValue(0.00);
        ui->combo_property_type->setCurrentIndex(0);
        ui->combo_property_status->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "失败", "保存失败: " + lastError());
    }
}
