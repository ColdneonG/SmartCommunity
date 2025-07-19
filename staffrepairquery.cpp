#include "staffrepairquery.h"
#include "ui_staffrepairquery.h"
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>
#include <QStyleFactory>
#include <QVBoxLayout>

StaffRepairQuery::StaffRepairQuery(int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StaffRepairQuery)
    , m_dbManager(new DatabaseManager(this))
    , tableWidget(nullptr)
    , m_currentUserId(userId)
{
    QString connName = QString("StaffRepairQuery_%1")
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

void StaffRepairQuery::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    // 使用布局管理器代替固定大小和位置
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableWidget);
    mainLayout->setContentsMargins(30, 290, 30, 30); // 设置边距

    tableWidget->setColumnCount(7);
    QStringList headers;
    headers << "报修编号" << "故障描述" << "报修时间" << "是否已处理" << "处理详情" << "处理时间" << "处理人";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void StaffRepairQuery::loadDataFromDatabase()
{
    tableWidget->clearContents();
    tableWidget->setRowCount(0);

    if (!m_dbManager->isOpen()) {
        qDebug() << "数据库未打开";
        QMessageBox::warning(this, "提示", "数据库未连接，无法加载数据");
        return;
    }

    // 查询所有报修单
    QString queryStr = "SELECT "
                    "ro.order_id, "
                    "ro.problem_desc, "
                    "ro.order_time, "
                    "rp.process_status, "
                    "rp.process_desc, "
                    "rp.process_time, "
                    "rp.staff_id "
                    "FROM repair_order ro "
                    "LEFT JOIN repair_progress rp ON ro.order_id = rp.order_id";

    // 执行查询
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr);

    // 检查查询结果是否有效
    if (result.isEmpty()) {
        // 检查数据库管理器的最后错误
        QString error = m_dbManager->lastError();
        if (!error.isEmpty()) {
            qDebug() << "查询失败:" << error;
            QMessageBox::warning(this, "查询失败", QString("无法获取报修单数据: %1").arg(error));
        } else {
            qDebug() << "查询成功但结果为空";
            QMessageBox::information(this, "提示", "没有找到任何报修单记录");
        }
        return;
    }

    // 处理查询结果
    for (const auto& row : result) {
        // 确保结果包含所有需要的字段
        if (!row.contains("order_id") || !row.contains("problem_desc") || !row.contains("order_time")) {
            qDebug() << "跳过无效的记录，缺少必要字段";
            continue;
        }

        int rowIndex = tableWidget->rowCount();
        tableWidget->insertRow(rowIndex);

        // 填充报修编号
        tableWidget->setItem(rowIndex, 0, new QTableWidgetItem(row["order_id"].toString()));

        // 填充故障描述
        tableWidget->setItem(rowIndex, 1, new QTableWidgetItem(row["problem_desc"].toString()));

        // 填充报修时间
        QString orderTimeStr = row["order_time"].toString().trimmed();
        tableWidget->setItem(rowIndex, 2, new QTableWidgetItem(orderTimeStr));

        // 填充是否已处理
        QString statusStr = row["process_status"].toString();
        bool isProcessed = false;
        if (statusStr.isEmpty()) {
            qDebug() << "process_status为空，订单ID:" << row["order_id"].toString();
        } else {
            isProcessed = (statusStr == "是");
        }
        tableWidget->setItem(rowIndex, 3, new QTableWidgetItem(getStatusText(isProcessed)));

        // 填充处理详情
        tableWidget->setItem(rowIndex, 4, new QTableWidgetItem(row["process_desc"].toString()));

        // 填充处理时间
        QString processTimeStr = row["process_time"].toString().trimmed(); // 去除可能的空格
        // 如果字段为空，显示空字符串；否则直接显示原始字符串
        tableWidget->setItem(rowIndex, 5, new QTableWidgetItem(processTimeStr));

        // 填充处理人
        tableWidget->setItem(rowIndex, 6, new QTableWidgetItem(row["staff_id"].toString()));
    }

    qDebug() << "数据加载完成，共" << tableWidget->rowCount() << "条记录";
}

QString StaffRepairQuery::getStatusText(bool isProcessed) const
{
    return isProcessed ? "是" : "否";
}

void StaffRepairQuery::on_refreshButton_clicked()
{
    loadDataFromDatabase();
}

StaffRepairQuery::~StaffRepairQuery()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}
