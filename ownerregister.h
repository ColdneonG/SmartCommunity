#ifndef OWNERREGISTER_H
#define OWNERREGISTER_H

#include <QWidget>
#include "databasemanager.h"  // 引入您的数据库管理类

QT_BEGIN_NAMESPACE
namespace Ui { class OwnerRegister; }
QT_END_NAMESPACE

class OwnerRegister : public QWidget
{
    Q_OBJECT

public:
    OwnerRegister(QWidget *parent = nullptr);
    ~OwnerRegister();

private slots:
    void on_registerButton_clicked();  // 注册按钮点击事件

private:
    Ui::OwnerRegister *ui;
    DatabaseManager *dbManager;  // 数据库管理器实例

    // 辅助函数
    bool validateInput();           // 验证输入合法性
    bool checkPasswordMatch();      // 检查两次密码是否一致
    void clearFormFields();         // 清空表单字段
};
#endif // OWNERREGISTER_H
