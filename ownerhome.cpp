#include "ownerhome.h"
#include "ui_ownerhome.h"
#include <QMessageBox>
#include <QDateTime>
#include <QVariantList>

OwnerHome::OwnerHome(int ownerID, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OwnerHome),
    m_ownerID(ownerID),
    m_selectedNoticeId(-1)
{
    ui->setupUi(this);
    setFixedSize(1360, 900);
    setWindowTitle("业主首页");

    m_dbManager = new DatabaseManager(this);
    setStyle();

    if (initDatabase()) {
        loadImportantInfo();
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "错误", "数据库连接失败");
    }

    connect(ui->noticeList, &QListWidget::itemClicked, this, &OwnerHome::on_noticeList_itemClicked);
}

OwnerHome::~OwnerHome()
{
    delete ui;
}

bool OwnerHome::initDatabase()
{
    QString connName = QString("OwnerHome_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    if (!m_dbManager->isOpen()) {
        return m_dbManager->initDatabase(connName);
    }
    return true;
}

void OwnerHome::setStyle()
{
    QString mainColor = "rgb(23, 64, 120)";
    QString style = QString(
        "QWidget { font-family: 'Microsoft YaHei'; }"
        "QLabel#titleLabel { color: %1; font-size: 24px; font-weight: bold; }"
        "QLabel#infoTitle, QLabel#noticeTitle, QLabel#noticeContentTitle { color: %1; font-size: 18px; font-weight: bold; }"
        "QListWidget::item:selected { background-color: %1; color: white; }"
        "QTextEdit { border: 1px solid %1; border-radius: 4px; padding: 5px; }"
    ).arg(mainColor);
    setStyleSheet(style);
}

void OwnerHome::loadImportantInfo()
{
    // 加载待缴费用：从 fee_log 表查询，筛选 owner_id 匹配且 payment_status 为 0（未缴费）的记录，求和 value 字段
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(
        "SELECT SUM(value) AS due_amount FROM fee_log WHERE owner_id = ? AND payment_status = 0",
        QVariantList() << m_ownerID
    );
    if (!result.isEmpty() && result.first().contains("due_amount") && !result.first()["due_amount"].isNull()) {
        // 将求和结果转为字符串，拼接“元”显示
        ui->duePaymentValue->setText(QString("%1元").arg(result.first()["due_amount"].toString()));
    } else {
        ui->duePaymentValue->setText("0元");
    }

    // 加载我的维修单数量：从 repair_order 表查询，筛选 owner_id 匹配的记录，统计数量
    result = m_dbManager->executeSelectQuery(
        "SELECT COUNT(*) AS count FROM repair_order WHERE owner_id = ?",
        QVariantList() << m_ownerID
    );
    if (!result.isEmpty()) {
        // 将统计的数量转为字符串，拼接“单”显示
        ui->maintenanceValue->setText(QString("%1单").arg(result.first()["count"].toString()));
    } else {
        ui->maintenanceValue->setText("0单");
    }
}
void OwnerHome::refreshNoticeList()
{
    ui->noticeList->clear();
    // 业主可以查看所有公告
    QVector<QVariantMap> notices = m_dbManager->executeSelectQuery(
        "SELECT id, title, time FROM announcement ORDER BY time DESC",
        QVariantList()
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

void OwnerHome::on_noticeList_itemClicked(QListWidgetItem *item)
{
    m_selectedNoticeId = item->data(Qt::UserRole).toInt();

    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(
        "SELECT title, announcement_content, time FROM announcement WHERE id = ?",
        QVariantList() << m_selectedNoticeId
    );

    if (!result.isEmpty()) {
        ui->selectedNoticeTitle->setText(result.first()["title"].toString());
        ui->noticeContent->setText(result.first()["announcement_content"].toString());
        ui->publishTimeLabel->setText(QString("发布时间：%1").arg(result.first()["time"].toString()));
    }
}
