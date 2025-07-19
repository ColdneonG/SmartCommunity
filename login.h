#ifndef LOGIN_H
#define LOGIN_H
#include <QDialog>
#include "databasemanager.h"  // 包含头文件

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    void clearInput();
signals:
    void loginSuccess(int, int);
private:
    Ui::Login *ui;
    DatabaseManager *dbManager;  // 新增：数据库管理器实例
private slots:
    void onLoginClicked();
};
#endif // LOGIN_H
