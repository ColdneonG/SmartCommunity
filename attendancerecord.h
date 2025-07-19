#ifndef ATTENDANCERECORD_H
#define ATTENDANCERECORD_H

#include <QWidget>
#include <QDateTime>
#include "databasemanager.h"

namespace Ui {
class AttendanceRecord;
}

class AttendanceRecord : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceRecord(int staffId, QWidget *parent = nullptr);
    ~AttendanceRecord() override;

private slots:
    // 上班打卡按钮点击事件
    void on_checkInButton_clicked();
    // 下班打卡按钮点击事件
    void on_checkOutButton_clicked();

private:
    // 初始化数据库连接
    void initDatabase();
    // 检查当天是否已上班打卡
    bool isCheckedInToday();
    // 检查当天是否已下班打卡
    bool isCheckedOutToday();
    // 更新打卡状态显示
    void updateStatusDisplay();
    // 格式化日期为"yyyy-MM-dd"
    QString formatDate(const QDate& date);
    // 格式化时间为"HH:mm:ss"
    QString formatTime(const QTime& time);

    // 成员变量
    Ui::AttendanceRecord *ui;         // UI对象
    int m_staffId;                    // 员工ID（构造时传入）
    QString m_currentDate;            // 当前日期
    QString m_checkInTime;            // 上班打卡时间
    QString m_checkOutTime;           // 下班打卡时间
    int m_status;                     // 打卡状态（0-未完成，1-正常，2-迟到早退）

    // 数据库管理器
    DatabaseManager* m_dbManager;
};

#endif // ATTENDANCERECORD_H
