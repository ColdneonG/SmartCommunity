#include "attendancerecord.h"
#include "ui_attendancerecord.h"
#include <QMessageBox>
#include <QTime>

AttendanceRecord::AttendanceRecord(int staffId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AttendanceRecord),
    m_staffId(staffId),
    m_status(0)
{
    ui->setupUi(this);

    // 初始化当前日期
    m_currentDate = formatDate(QDate::currentDate());

    // 设置员工ID和日期显示
    ui->staffIdValueLabel->setText(QString::number(m_staffId));
    ui->attendanceDateValueLabel->setText(m_currentDate);

    // 初始化数据库
    initDatabase();

    // 检查当天打卡状态
    updateStatusDisplay();
}

AttendanceRecord::~AttendanceRecord()
{
    delete ui;
    delete m_dbManager;
}

void AttendanceRecord::on_checkInButton_clicked()
{
    // 检查是否已打卡
    if (isCheckedInToday()) {
        QMessageBox::information(this, "提示", "今日已完成上班打卡，请勿重复操作");
        return;
    }

    // 获取当前时间
    QTime currentTime = QTime::currentTime();
    m_checkInTime = formatTime(currentTime);

    // 执行插入操作
    QString sql = "INSERT INTO attendance (staff_id, attendance_date, check_in_time, status) "
                  "VALUES (?, ?, ?, ?)";
    QVariantList bindValues;
    bindValues << m_staffId << m_currentDate << m_checkInTime << m_status;

    if (m_dbManager->executeQuery(sql, bindValues)) {
        QMessageBox::information(this, "成功", "上班打卡成功");
        ui->checkInTimeValueLabel->setText(m_checkInTime);
        updateStatusDisplay();
    } else {
        QMessageBox::critical(this, "失败", "打卡失败: " + m_dbManager->lastError());
    }
}

void AttendanceRecord::on_checkOutButton_clicked()
{
    // 检查是否已上班打卡
    if (!isCheckedInToday()) {
        QMessageBox::warning(this, "提示", "请先完成上班打卡");
        return;
    }

    // 检查是否已下班打卡
    if (isCheckedOutToday()) {
        QMessageBox::information(this, "提示", "今日已完成下班打卡，请勿重复操作");
        return;
    }

    // 获取当前时间
    QTime currentTime = QTime::currentTime();
    m_checkOutTime = formatTime(currentTime);

    // 查询上班打卡时间
    QString querySql = "SELECT check_in_time FROM attendance "
                       "WHERE staff_id = ? AND attendance_date = ?";
    QVariantList queryValues;
    queryValues << m_staffId << m_currentDate;
    auto result = m_dbManager->executeSelectQuery(querySql, queryValues);

    if (result.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到上班打卡记录");
        return;
    }

    // 解析上班时间
    QString checkInTimeStr = result.first()["check_in_time"].toString();
    QTime checkInTime = QTime::fromString(checkInTimeStr, "HH:mm:ss");
    QTime checkOutTime = currentTime;

    // 判断考勤状态
    bool isLate = checkInTime > QTime(9, 0);          // 迟到（9点后打卡）
    bool isEarlyLeave = checkOutTime < QTime(17, 0);  // 早退（17点前打卡）
    m_status = (isLate || isEarlyLeave) ? 2 : 1;

    // 执行更新操作
    QString updateSql = "UPDATE attendance SET check_out_time = ?, status = ? "
                       "WHERE staff_id = ? AND attendance_date = ?";
    QVariantList updateValues;
    updateValues << m_checkOutTime << m_status << m_staffId << m_currentDate;

    if (m_dbManager->executeQuery(updateSql, updateValues)) {
        QMessageBox::information(this, "成功", "下班打卡成功");
        ui->checkOutTimeValueLabel->setText(m_checkOutTime);
        updateStatusDisplay();
    } else {
        QMessageBox::critical(this, "失败", "打卡失败: " + m_dbManager->lastError());
    }
}

void AttendanceRecord::initDatabase()
{
    m_dbManager = new DatabaseManager(this);

    QString connName = QString("AttendanceRecord_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    if (!m_dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", "无法连接数据库: " + m_dbManager->lastError());
    }
}

bool AttendanceRecord::isCheckedInToday()
{
    QString sql = "SELECT * FROM attendance "
                 "WHERE staff_id = ? AND attendance_date = ? AND check_in_time IS NOT NULL";
    QVariantList values;
    values << m_staffId << m_currentDate;
    auto result = m_dbManager->executeSelectQuery(sql, values);
    return !result.isEmpty();
}

bool AttendanceRecord::isCheckedOutToday()
{
    QString sql = "SELECT * FROM attendance "
                 "WHERE staff_id = ? AND attendance_date = ? AND check_out_time IS NOT NULL";
    QVariantList values;
    values << m_staffId << m_currentDate;
    auto result = m_dbManager->executeSelectQuery(sql, values);
    return !result.isEmpty();
}

void AttendanceRecord::updateStatusDisplay()
{
    // 查询当前考勤状态
    QString sql = "SELECT check_in_time, check_out_time, status FROM attendance "
                 "WHERE staff_id = ? AND attendance_date = ?";
    QVariantList values;
    values << m_staffId << m_currentDate;
    auto result = m_dbManager->executeSelectQuery(sql, values);

    if (!result.isEmpty()) {
        auto record = result.first();
        m_checkInTime = record["check_in_time"].toString();
        m_checkOutTime = record["check_out_time"].toString();
        m_status = record["status"].toInt();

        // 更新UI显示
        ui->checkInTimeValueLabel->setText(m_checkInTime.isEmpty() ? "未打卡" : m_checkInTime);
        ui->checkOutTimeValueLabel->setText(m_checkOutTime.isEmpty() ? "未打卡" : m_checkOutTime);
    }
}

QString AttendanceRecord::formatDate(const QDate& date)
{
    return date.toString("yyyy-MM-dd");
}

QString AttendanceRecord::formatTime(const QTime& time)
{
    return time.toString("HH:mm:ss");
}
