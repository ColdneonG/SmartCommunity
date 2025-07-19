#include "ownerregister.h"
#include "ui_ownerregister.h"
#include "familymember.h"
#include <QMessageBox>
#include <QDateTime>  // 获取系统当前时间

OwnerRegister::OwnerRegister(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OwnerRegister)
{
    QString connName = QString("OwnerRegister_%1")
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

OwnerRegister::~OwnerRegister()
{
    dbManager->closeDatabase();  // 关闭数据库连接
    delete ui;
}

bool OwnerRegister::validateInput()
{
    // 检查必填字段
    if (ui->nameLineEdit->text().isEmpty() ||
        ui->idLineEdit->text().isEmpty() ||
        ui->phoneLineEdit->text().isEmpty() ||
        ui->emailLineEdit->text().isEmpty() ||
        ui->usernameLineEdit->text().isEmpty() ||
        ui->passwordLineEdit->text().isEmpty() ||
        ui->confirmPasswordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写所有必填字段！");
        return false;
    }

    // 验证邮箱格式
    QRegularExpression emailRegex("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}");
    if (!emailRegex.match(ui->emailLineEdit->text()).hasMatch()) {
        QMessageBox::warning(this, "输入错误", "请输入有效的电子邮箱地址！");
        return false;
    }

    // 验证手机号格式
    QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
    if (!phoneRegex.match(ui->phoneLineEdit->text()).hasMatch()) {
        QMessageBox::warning(this, "输入错误", "请输入有效的手机号码！");
        return false;
    }

    return true;
}

bool OwnerRegister::checkPasswordMatch()
{
    if (ui->passwordLineEdit->text() != ui->confirmPasswordLineEdit->text()) {
        QMessageBox::warning(this, "密码错误", "两次输入的密码不一致！");
        return false;
    }
    return true;
}

void OwnerRegister::clearFormFields()
{
    // 清空所有输入字段
    ui->nameLineEdit->clear();
    ui->idLineEdit->clear();
    ui->phoneLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->checkinDateEdit->setDate(QDate::currentDate());
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();
}

void OwnerRegister::on_registerButton_clicked()
{
    // 1. 验证输入
    if (!validateInput() || !checkPasswordMatch()) {
        return;
    }

    // 2. 获取输入值
    QString name = ui->nameLineEdit->text();
    QString idNumber = ui->idLineEdit->text();
    QString phone = ui->phoneLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString moveInDate = ui->checkinDateEdit->date().toString("yyyy-MM-dd");
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString registerTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 3. 开始数据库事务
    if (!dbManager->beginTransaction()) {
        QMessageBox::critical(this, "事务错误", "无法开始数据库事务: " + dbManager->lastError());
        return;
    }

    // 4. 插入 owner_info 表
    QString ownerQuery = "INSERT INTO owner_info (name, id_number, phone_number, email, move_in_date, register_time) "
                         "VALUES (:name, :id_number, :phone_number, :email, :move_in_date, :register_time)";
    QVariantList ownerValues = {name, idNumber, phone, email, moveInDate, registerTime};

    if (!dbManager->executeQuery(ownerQuery, ownerValues)) {
        dbManager->rollbackTransaction();
        return;
    }

    // 5. 插入 users 表（role=2 表示业主）
    QString userQuery = "INSERT INTO users (username, password, role) "
                        "VALUES (:username, :password, 2)";
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
        QMessageBox::information(this, "注册成功", "业主信息已成功注册！");
        clearFormFields();  // 清空表单
        FamilyMember *familyMemberWindow = new FamilyMember(userId, dbManager, this);
        familyMemberWindow->show();
        this->hide();
    } else {
        QMessageBox::critical(this, "事务错误", "提交事务失败: " + dbManager->lastError());
    }
}
