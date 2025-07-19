#include "addstaff.h"
#include "ui_addstaff.h"
#include <QMessageBox>
#include <QDateTime>

AddStaff::AddStaff(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddStaff)
{
    QString connName = QString("AddStaff_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    dbManager = new DatabaseManager(this);  // 创建数据库管理器实例

    // 连接数据库初始化信号
    connect(dbManager, &DatabaseManager::databaseOpened, this, [=](bool success, const QString& message) {
        if (!success) {
            QMessageBox::critical(this, "数据库错误", "初始化失败: " + message);
        }
    });

    // 连接数据库查询信号
    connect(dbManager, &DatabaseManager::queryExecuted, this, [=](bool success, const QString& message) {
        if (!success) {
            QMessageBox::critical(this, "数据库操作失败", message);
        }
    });

    // 初始化数据库
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", dbManager->lastError());
    }
}

AddStaff::~AddStaff()
{
    dbManager->closeDatabase();  // 关闭数据库连接
    delete ui;
}

bool AddStaff::validateInput()
{
    // 检查必填字段
    if (ui->usernameLineEdit->text().isEmpty() ||
        ui->passwordLineEdit->text().isEmpty() ||
        ui->confirmPasswordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写所有必填字段！");
        return false;
    }
    return true;
}

bool AddStaff::checkPasswordMatch()
{
    if (ui->passwordLineEdit->text() != ui->confirmPasswordLineEdit->text()) {
        QMessageBox::warning(this, "密码错误", "两次输入的密码不一致！");
        return false;
    }
    return true;
}

void AddStaff::clearFormFields()
{
    // 清空所有输入字段
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();
}

void AddStaff::on_addButton_clicked()
{
    if (!validateInput() || !checkPasswordMatch()) {
        return;
    }

    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (!dbManager->beginTransaction()) {
        QMessageBox::critical(this, "事务错误", "无法开始数据库事务: " + dbManager->lastError());
        return;
    }
    QString userQuery = "INSERT INTO users (username, password, role) "
                        "VALUES (:username, :password, 1)";
    QVariantList userValues = {username, password};
    if (!dbManager->executeQuery(userQuery, userValues)) {
        dbManager->rollbackTransaction();
        return;
    }

    int userId = -1;
    QVector<QVariantMap> result = dbManager->executeSelectQuery(
        "SELECT last_insert_rowid() AS user_id", {});
    if (!result.isEmpty()) {
        userId = result.first().value("user_id").toInt();
    } else {
        dbManager->rollbackTransaction();
        QMessageBox::critical(this, "数据库错误", "无法获取用户ID");
        return;
    }

    // 6. 提交事务
    if (dbManager->commitTransaction()) {
        QMessageBox::information(this, "添加成功", "员工信息已成功添加！");
        clearFormFields();  // 清空表单
        emit addSuccess(userId);
    } else {
        QMessageBox::critical(this, "事务错误", "提交事务失败: " + dbManager->lastError());
    }
}
