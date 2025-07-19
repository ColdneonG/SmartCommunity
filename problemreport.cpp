#include "problemreport.h"
#include "ui_problemreport.h"
#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QMessageBox>

ProblemReport::ProblemReport(int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProblemReport)
    , owner_id(userId)
    , m_dbManager(new DatabaseManager(this)) // 独立管理数据库连接
{
    ui->setupUi(this);
    // 设置无边框
    setWindowFlags(Qt::FramelessWindowHint);
    // 去掉背景透明
    setAttribute(Qt::WA_TranslucentBackground, false);

    // 初始化数据库连接
    if (!initDatabaseConnection()) {
        qDebug() << "数据库初始化失败";
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }
}

ProblemReport::~ProblemReport()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase(); // 析构时才关闭连接
    }
    delete ui;
}

//初始化并检查数据库连接
bool ProblemReport::initDatabaseConnection()
{
    // 若已连接则直接返回
    if (m_dbManager->isOpen()) {
        return true;
    }
    QString connName = QString("ProblemReport_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    // 重新初始化连接
    return m_dbManager->initDatabase(connName);
}

void ProblemReport::on_pushButton_clicked()
{
    QString problemDescription = ui->textEdit->toPlainText();
    if (problemDescription.isEmpty()) {
        qDebug() << "故障描述不能为空";
        QMessageBox::information(this, "提示", "请输入故障描述");
        return;
    }

    // 关键修改：操作前检查连接状态（参考RepairEvaluate的检查逻辑）
    if (!initDatabaseConnection()) {
        qDebug() << "数据库连接失败，无法提交报修";
        QMessageBox::critical(this, "错误", "数据库连接失败，无法提交报修");
        return;
    }

    QString insertQuery = "INSERT INTO repair_order (problem_desc, owner_id, order_time) VALUES (?, ?, ?)";
    QVariantList bindValues;
    bindValues.append(problemDescription);
    bindValues.append(owner_id);
    bindValues.append(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    // 执行查询
    if (m_dbManager->executeQuery(insertQuery, bindValues)) {
        qDebug() << "故障报修信息已成功提交到数据库";
        ui->textEdit->clear();
        QMessageBox::information(this, "成功", "故障报修已提交");
    } else {
        qDebug() << "提交故障报修信息时出现错误: " << m_dbManager->lastError();
        QMessageBox::critical(this, "错误", "提交失败：" + m_dbManager->lastError());
    }
}
