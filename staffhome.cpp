#include "staffhome.h"
#include "ui_staffhome.h"
#include <QMessageBox>
#include <QDateTime>
#include <QVariantList>

StaffHome::StaffHome(int staffID, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StaffHome)
    , m_staffID(staffID)
    , m_selectedNoticeId(-1)
{
    ui->setupUi(this);
    setFixedSize(1360, 900);
    setWindowTitle("物业工作人员首页");

    m_dbManager = new DatabaseManager(this);
    setStyle();

    if (initDatabase()) {
        loadImportantInfo();
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "数据库连接失败");
    }
}

StaffHome::~StaffHome()
{
    delete ui;
}

bool StaffHome::initDatabase()
{
    QString connName = QString("StaffHome_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    if (!m_dbManager->isOpen()) {
        return m_dbManager->initDatabase(connName);
    }
    return true;
}

void StaffHome::setStyle()
{
    QString mainColor = "rgb(23, 64, 120)";
    QString style = QString(
        "QWidget { font-family: 'Microsoft YaHei'; }"
        "QLabel#titleLabel { color: %1; font-size: 24px; font-weight: bold; }"
        "QLabel#infoTitle, QLabel#noticeTitle, QLabel#editTitle { color: %1; font-size: 18px; font-weight: bold; }"
        "QPushButton { background-color: %1; color: white; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: rgb(33, 74, 130); }"
        "QPushButton:pressed { background-color: rgb(13, 54, 100); }"
        "QListWidget::item:selected { background-color: %1; color: white; }"
        "QLineEdit, QTextEdit { border: 1px solid %1; border-radius: 4px; padding: 5px; }"
    ).arg(mainColor);
    setStyleSheet(style);
}

void StaffHome::loadImportantInfo()
{
    QVector<QVariantMap> result;

    // 未处理维修单（自己负责的）
    result = m_dbManager->executeSelectQuery(
        "SELECT COUNT(*) AS count FROM repair_order WHERE handler_id = ? AND status = 0",
        QVariantList() << m_staffID
    );
    if (!result.isEmpty()) {
        ui->untreatedRepair->setText(result.first()["count"].toString());
    }

    // 已完成维修单
    result = m_dbManager->executeSelectQuery(
        "SELECT COUNT(*) AS count FROM repair_order WHERE handler_id = ? AND status = 1",
        QVariantList() << m_staffID
    );
    if (!result.isEmpty()) {
        ui->completedRepair->setText(result.first()["count"].toString());
    }

    // 我的公告数量
    result = m_dbManager->executeSelectQuery(
        "SELECT COUNT(*) AS count FROM announcement WHERE operator_id = ?",
        QVariantList() << m_staffID
    );
    if (!result.isEmpty()) {
        ui->myNoticeCount->setText(result.first()["count"].toString());
    }
}

void StaffHome::refreshNoticeList()
{
    ui->noticeList->clear();
    QVector<QVariantMap> notices = m_dbManager->executeSelectQuery(
        "SELECT id, title, time FROM announcement WHERE operator_id = ? ORDER BY time DESC",
        QVariantList() << m_staffID
    );

    foreach (const QVariantMap &notice, notices) {
        QString itemText = QString("ID:%1 %2 【发布时间：%3】")
            .arg(notice["id"].toString())
            .arg(notice["title"].toString())
            .arg(notice["time"].toString());
        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, notice["id"]);
        ui->noticeList->addItem(item);
    }
}

void StaffHome::on_publishBtn_clicked()
{
    QString title = ui->noticeTitleEdit->text().trimmed();
    QString content = ui->noticeContentEdit->toPlainText().trimmed();

    if (title.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "提示", "公告标题和内容不能为空");
        return;
    }

    QString sql = "INSERT INTO announcement (title, announcement_content, operator_id, time) VALUES (?, ?, ?, ?)";
    QVariantList values;
    values << title << content << m_staffID
           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告发布成功");
        ui->noticeTitleEdit->clear();
        ui->noticeContentEdit->clear();
        refreshNoticeList();
        loadImportantInfo();
    } else {
        QMessageBox::critical(this, "错误", "发布失败: " + m_dbManager->lastError());
    }
}

void StaffHome::on_modifyBtn_clicked()
{
    if (m_selectedNoticeId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要修改的公告");
        return;
    }

    QString title = ui->noticeTitleEdit->text().trimmed();
    QString content = ui->noticeContentEdit->toPlainText().trimmed();

    if (title.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "提示", "公告标题和内容不能为空");
        return;
    }

    QString sql = "UPDATE announcement SET title = ?, announcement_content = ?, time = ? WHERE id = ? AND operator_id = ?";
    QVariantList values;
    values << title << content
           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
           << m_selectedNoticeId << m_staffID;

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告修改成功");
        ui->noticeTitleEdit->clear();
        ui->noticeContentEdit->clear();
        m_selectedNoticeId = -1;
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "修改失败: " + m_dbManager->lastError());
    }
}

void StaffHome::on_deleteBtn_clicked()
{
    if (m_selectedNoticeId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要删除的公告");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除此公告吗？") != QMessageBox::Yes) {
        return;
    }

    QString sql = "DELETE FROM announcement WHERE id = ? AND operator_id = ?";
    QVariantList values;
    values << m_selectedNoticeId << m_staffID;

    if (m_dbManager->executeQuery(sql, values)) {
        QMessageBox::information(this, "成功", "公告删除成功");
        ui->noticeTitleEdit->clear();
        ui->noticeContentEdit->clear();
        m_selectedNoticeId = -1;
        refreshNoticeList();
        loadImportantInfo();
    } else {
        QMessageBox::critical(this, "错误", "删除失败: " + m_dbManager->lastError());
    }
}

void StaffHome::on_noticeList_itemClicked(QListWidgetItem *item)
{
    m_selectedNoticeId = item->data(Qt::UserRole).toInt();
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(
        "SELECT title, announcement_content FROM announcement WHERE id = ? AND operator_id = ?",
        QVariantList() << m_selectedNoticeId << m_staffID
    );

    if (!result.isEmpty()) {
        ui->noticeTitleEdit->setText(result.first()["title"].toString());
        ui->noticeContentEdit->setText(result.first()["announcement_content"].toString());
    }
}
