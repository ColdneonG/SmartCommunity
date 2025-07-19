#ifndef LEAVEAPPLY_H
#define LEAVEAPPLY_H

#include <QWidget>
#include <QDateTime>
#include "databasemanager.h"

namespace Ui {
class LeaveApply;
}

class LeaveApply : public QWidget
{
    Q_OBJECT

public:
    explicit LeaveApply(int staffID, QWidget *parent = nullptr);
    ~LeaveApply();

private slots:
    void on_submitButton_clicked();

private:
    Ui::LeaveApply *ui;
    DatabaseManager *dbManager;
    int m_staffID; // 存储申请人ID（来自构造函数参数）

    bool validateInput(); // 验证输入合法性
    void clearFormFields(); // 清空表单字段
};

#endif // LEAVEAPPLY_H
