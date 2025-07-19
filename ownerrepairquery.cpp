#include "ownerrepairquery.h"
#include "ui_ownerrepairquery.h"
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>

OwnerRepairQuery::OwnerRepairQuery(int ownerId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OwnerRepairQuery)
    , m_dbManager(new DatabaseManager(this))
    , tableWidget(nullptr)
    , m_currentOwnerId(ownerId)
{
    QString connName = QString("OwnerRepairQuery_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    // 设置无边框
    setWindowFlags(Qt::FramelessWindowHint);
    // 去掉背景透明
    setAttribute(Qt::WA_TranslucentBackground, false);

    if (!m_dbManager->initDatabase(connName)) {
        qDebug() << "数据库初始化失败";
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }

    initTableWidget();
    loadDataFromDatabase(); // 初始加载数据
}

void OwnerRepairQuery::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    // 使用布局管理器代替固定大小和位置
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableWidget);
    mainLayout->setContentsMargins(30, 290, 30, 30); // 保持边距风格一致

    tableWidget->setColumnCount(5);
    QStringList headers;
    headers << "报修时间" << "故障描述" << "是否已处理" << "处理详情" << "处理时间";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void OwnerRepairQuery::loadDataFromDatabase()
{
    tableWidget->clearContents();
    tableWidget->setRowCount(0);

    if (!m_dbManager->isOpen()) {
        qDebug() << "数据库未打开";
        return;
    }

    QString queryStr = "SELECT "
                      "ro.order_time, "
                      "ro.problem_desc, "
                      "rp.process_status, "
                      "rp.process_desc, "
                      "rp.process_time "
                      "FROM repair_order ro "
                      "LEFT JOIN repair_progress rp ON ro.order_id = rp.order_id "
                      "WHERE ro.owner_id = ?";

    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr, {QVariant(m_currentOwnerId)});

    for (const auto& row : result) {
        int rowIndex = tableWidget->rowCount();
        tableWidget->insertRow(rowIndex);

        QDateTime orderTime = row["order_time"].toDateTime();
        tableWidget->setItem(rowIndex, 0, new QTableWidgetItem(orderTime.toString("yyyy-MM-dd hh:mm:ss")));

        tableWidget->setItem(rowIndex, 1, new QTableWidgetItem(row["problem_desc"].toString()));

        bool isProcessed = row["process_status"].toBool();
        tableWidget->setItem(rowIndex, 2, new QTableWidgetItem(getStatusText(isProcessed)));

        tableWidget->setItem(rowIndex, 3, new QTableWidgetItem(row["process_desc"].toString()));

        QDateTime processTime = row["process_time"].toDateTime();
        tableWidget->setItem(rowIndex, 4, new QTableWidgetItem(processTime.isValid() ?
                                                         processTime.toString("yyyy-MM-dd hh:mm:ss") : ""));
    }

    qDebug() << "业主报修数据加载完成，共" << tableWidget->rowCount() << "条记录";
}

QString OwnerRepairQuery::getStatusText(bool isProcessed) const
{
    return isProcessed ? "是" : "否";
}

void OwnerRepairQuery::on_refreshButton_clicked()
{
    loadDataFromDatabase();
}

OwnerRepairQuery::~OwnerRepairQuery()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}
