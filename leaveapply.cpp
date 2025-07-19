#include "leaveapply.h"
#include "ui_leaveapply.h"
#include <QMessageBox>
#include <QDateTime>

LeaveApply::LeaveApply(int staffID, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeaveApply),
    m_staffID(staffID)
{
    QString connName = QString("LeaveApply_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    dbManager = new DatabaseManager(this); // 创建数据库管理器实例

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

LeaveApply::~LeaveApply()
{
    dbManager->closeDatabase(); // 关闭数据库连接
    delete ui;
}

bool LeaveApply::validateInput()
{
    // 检查开始日期和结束日期是否为空（QDateEdit默认有当前日期，此处检查是否为有效日期）
    if (!ui->startDateEdit->date().isValid() || !ui->endDateEdit->date().isValid()) {
        QMessageBox::warning(this, "输入错误", "请选择有效的开始日期和结束日期！");
        return false;
    }

    // 检查开始日期是否晚于结束日期
    if (ui->startDateEdit->date() > ui->endDateEdit->date()) {
        QMessageBox::warning(this, "输入错误", "开始日期不能晚于结束日期！");
        return false;
    }

    // 检查请假原因是否为空
    if (ui->reasonTextEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写请假原因！");
        return false;
    }

    return true;
}

void LeaveApply::clearFormFields()
{
    // 清空表单字段（日期重置为当前日期，原因清空）
    ui->startDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDate(QDate::currentDate());
    ui->reasonTextEdit->clear();
}

void LeaveApply::on_submitButton_clicked()
{
    // 1. 验证输入合法性
    if (!validateInput()) {
        return;
    }

    // 2. 获取输入值
    QString startDate = ui->startDateEdit->date().toString("yyyy-MM-dd");
    QString endDate = ui->endDateEdit->date().toString("yyyy-MM-dd");
    QString reason = ui->reasonTextEdit->toPlainText().trimmed();
    QString applyTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 3. 开始数据库事务
    if (!dbManager->beginTransaction()) {
        QMessageBox::critical(this, "事务错误", "无法开始数据库事务: " + dbManager->lastError());
        return;
    }

    // 4. 插入请假记录到leave_applications表
    QString queryStr = "INSERT INTO leave_applications "
                       "(applicant_id, start_date, end_date, reason, apply_time) "
                       "VALUES (:applicant_id, :start_date, :end_date, :reason, :apply_time)";

    QVariantList values = {
        m_staffID,       // applicant_id
        startDate,       // start_date
        endDate,         // end_date
        reason,          // reason
        applyTime        // apply_time
    };

    if (!dbManager->executeQuery(queryStr, values)) {
        dbManager->rollbackTransaction(); // 执行失败回滚事务
        return;
    }

    // 5. 提交事务
    if (dbManager->commitTransaction()) {
        QMessageBox::information(this, "提交成功", "请假申请已成功提交！");
        clearFormFields(); // 清空表单
    } else {
        QMessageBox::critical(this, "事务错误", "提交事务失败: " + dbManager->lastError());
        dbManager->rollbackTransaction();
    }
}
