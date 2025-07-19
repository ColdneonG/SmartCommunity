#include "login.h"
#include "ui_login.h"
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>

Login::Login(QWidget* parent) : QDialog(parent), ui(new Ui::Login), dbManager(nullptr) {
    QString connName = QString("Login_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    ui->LE_password->setEchoMode(QLineEdit::Password);

    //创建DatabaseManager实例
    dbManager = new DatabaseManager(this);  // 父对象为this，自动管理生命周期

    //手动初始化数据库
    if (!dbManager->initDatabase(connName)) {  // 调用initDatabase打开数据库
        QMessageBox::critical(this, "错误", "数据库打开失败！" + dbManager->lastError());
    }

    connect(ui->pushButton, &QPushButton::clicked, this, &Login::onLoginClicked);
}

void Login::onLoginClicked() {
    QString username = ui->LE_username->text().trimmed();
    QString password = ui->LE_password->text().trimmed();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码!");
        return;
    }

    if (!dbManager->isOpen()) {
        QString connName = QString("Login_%1")
                   .arg(QDateTime::currentMSecsSinceEpoch());
        if (!dbManager->initDatabase(connName)) {
            QMessageBox::critical(this, "错误", "数据库打开失败！");
            return;
        }
    }

    //使用dbManager实例执行查询
    QVariantList params;
    params << username;
    QVector<QVariantMap> results = dbManager->executeSelectQuery(
        "SELECT id, password, role FROM users WHERE username = ?",
        params
    );

    if (!results.isEmpty()) {
        QVariantMap user = results.first();
        QString dbPassword = user["password"].toString();
        if (dbPassword != password) {
            QMessageBox::warning(this, "登录失败", "密码错误!");
            return;
        }
        int userId = user["id"].toInt();
        int role = user["role"].toInt();
        emit loginSuccess(userId, role);
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "用户不存在,请先注册!");
    }
}

void Login::clearInput() {
    ui->LE_username->clear();
    ui->LE_password->clear();
}

Login::~Login() {
    delete ui;
}
