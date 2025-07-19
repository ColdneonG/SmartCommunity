#include "adminhome.h"
#include "ui_adminhome.h"
#include <QMessageBox>
#include <QDateTime>

AdminHome::AdminHome(int staffID, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminHome)
    , m_selectedNoticeId(-1)
    , m_staffID(staffID) // 初始化 staffID
{
    ui->setupUi(this);
    m_dbManager = new DatabaseManager(this);

    setFixedSize(1360, 900);
    setWindowTitle("管理员首页");

    setStyle();

    if (initDatabase()) {
        loadImportantInfo();
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "数据库连接失败");
    }
}

AdminHome::~AdminHome()
{
    delete ui;
}

bool AdminHome::initDatabase()
{
    QString connName = QString("AdminHome_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    if (!m_dbManager->isOpen()) {
        return m_dbManager->initDatabase(connName);
    }
    return true;
}

void AdminHome::loadImportantInfo()
{
    QVector<QVariantMap> result;
    // 统计 users 表中不同 role 的数量，role 0:管理员，1:物业工作人员，2:业主
    // 管理员数量
    result = m_dbManager->executeSelectQuery("SELECT COUNT(*) AS count FROM users WHERE role = 0");
    if (!result.isEmpty()) {
        ui->adminCount->setText(result.first()["count"].toString());
    }
    // 物业工作人员数量
    result = m_dbManager->executeSelectQuery("SELECT COUNT(*) AS count FROM users WHERE role = 1");
    if (!result.isEmpty()) {
        ui->staffCount->setText(result.first()["count"].toString());
    }
    // 业主数量
    result = m_dbManager->executeSelectQuery("SELECT COUNT(*) AS count FROM users WHERE role = 2");
    if (!result.isEmpty()) {
        ui->ownerCount->setText(result.first()["count"].toString());
    }

    // 统计待审批请假（leave_applications 表中 approval_status 为待审批的数量）
    result = m_dbManager->executeSelectQuery("SELECT COUNT(*) AS count FROM leave_applications WHERE approval_status = '待审批'");
    if (!result.isEmpty()) {
        ui->pendingLeaveCount->setText(result.first()["count"].toString());
    }
}

void AdminHome::refreshNoticeList()
{
    ui->noticeList->clear();

    QVector<QVariantMap> notices = m_dbManager->executeSelectQuery(
        "SELECT id, title, create_time FROM announcement ORDER BY create_time DESC"
    );

    foreach (const QVariantMap &notice, notices) {
        QString itemText = QString("ID:%1 %2 【发布时间：%3】")
            .arg(notice["id"].toString())
            .arg(notice["title"].toString())
            .arg(notice["create_time"].toString());

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, notice["id"]);
        ui->noticeList->addItem(item);
    }
}

void AdminHome::setStyle()
{
    QString mainColor = "rgb(23, 64, 120)";

    QString style = QString(
        "QWidget { font-family: 'Microsoft YaHei'; }"
        "QLabel#titleLabel { color: %1; font-size: 24px; font-weight: bold; }"
        "QLabel#infoTitle, QLabel#noticeTitle { color: %1; font-size: 18px; font-weight: bold; }"
        "QPushButton { background-color: %1; color: white; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: rgb(33, 74, 130); }"
        "QPushButton:pressed { background-color: rgb(13, 54, 100); }"
        "QListWidget::item:selected { background-color: %1; color: white; }"
    ).arg(mainColor);

    setStyleSheet(style);
}

void AdminHome::on_publishBtn_clicked()
{
    QString title = ui->noticeTitleEdit->text().trimmed();
    QString content = ui->noticeContent->toPlainText().trimmed();

    if (title.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "提示", "公告标题和内容不能为空");
        return;
    }

    QString sql = "INSERT INTO announcement (title, announcement_content, operator_id, time) VALUES (?, ?, ?, ?)";
    QVariantList values;
    values << title
           << content
           << m_staffID // 使用构造函数传入的 staffID 作为发布人 ID
           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"); // 自动获取当前时间

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告发布成功");
        ui->noticeTitle->clear();
        ui->noticeContent->clear();
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "发布失败: " + m_dbManager->lastError());
    }
}

void AdminHome::on_modifyBtn_clicked()
{
    if (m_selectedNoticeId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要修改的公告");
        return;
    }

    QString title = ui->noticeTitle->text().trimmed();
    QString content = ui->noticeContent->toPlainText().trimmed();

    if (title.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "提示", "公告标题和内容不能为空");
        return;
    }

    QString sql = "UPDATE announcement SET title = ?, announcement_content = ?, time = ? WHERE id = ?";
    QVariantList values;
    values << title
           << content
           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
           << m_selectedNoticeId;

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告修改成功");
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "修改失败: " + m_dbManager->lastError());
    }
}

void AdminHome::on_deleteBtn_clicked()
{
    if (m_selectedNoticeId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要删除的公告");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除此公告吗？") != QMessageBox::Yes) {
        return;
    }

    QString sql = "DELETE FROM announcement WHERE id = ?";
    QVariantList values;
    values << m_selectedNoticeId;

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告删除成功");
        ui->noticeTitle->clear();
        ui->noticeContent->clear();
        m_selectedNoticeId = -1;
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "删除失败: " + m_dbManager->lastError());
    }
}

void AdminHome::on_noticeList_itemClicked(QListWidgetItem *item)
{
    m_selectedNoticeId = item->data(Qt::UserRole).toInt();

    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(
        "SELECT title, announcement_content FROM announcement WHERE id = ?", // 表名和字段名适配
        QVariantList() << m_selectedNoticeId
    );

    if (!result.isEmpty()) {
        ui->noticeTitle->setText(result.first()["title"].toString());
        ui->noticeContent->setText(result.first()["announcement_content"].toString());
    }
}
