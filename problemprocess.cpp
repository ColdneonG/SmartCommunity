#include "problemprocess.h"
#include "ui_problemprocess.h"
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

ProblemProcess::ProblemProcess(int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProblemProcess)
    , m_dbManager(new DatabaseManager(this))
    , tableWidget(nullptr)
    , m_currentUserId(userId)
{
    QString connName = QString("ProblemProcess_%1")
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

void ProblemProcess::initTableWidget()
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

void ProblemProcess::loadDataFromDatabase()
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
        QDateTime orderTime = row["order_time"].toDateTime();
        tableWidget->setItem(rowIndex, 2, new QTableWidgetItem(orderTime.toString("yyyy-MM-dd hh:mm:ss")));

        // 填充是否已处理
        bool isProcessed = row["process_status"].toBool();
        tableWidget->setItem(rowIndex, 3, new QTableWidgetItem(getStatusText(isProcessed)));

        // 填充处理详情
        tableWidget->setItem(rowIndex, 4, new QTableWidgetItem(row["process_desc"].toString()));

        // 填充处理时间
        QDateTime processTime = row["process_time"].toDateTime();
        tableWidget->setItem(rowIndex, 5, new QTableWidgetItem(processTime.isValid() ?
                                                         processTime.toString("yyyy-MM-dd hh:mm:ss") : ""));

        // 填充处理人
        tableWidget->setItem(rowIndex, 6, new QTableWidgetItem(row["staff_id"].toString()));
    }

    qDebug() << "数据加载完成，共" << tableWidget->rowCount() << "条记录";
}

QString ProblemProcess::getStatusText(bool isProcessed) const
{
    return isProcessed ? "是" : "否";
}

void ProblemProcess::on_refreshButton_clicked()
{
    qDebug() << "refreshButton clicked from:" << this;
    loadDataFromDatabase();
}

void ProblemProcess::on_processButton_clicked()
{
    qDebug() << "processButton clicked from:" << this;
    // 获取选中行
    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要处理的报修单");
        return;
    }

    // 获取选中行的行号
    int row = selectedItems.first()->row();

    // 检查是否已处理
    QTableWidgetItem* statusItem = tableWidget->item(row, 3);
    if (statusItem && statusItem->text() == "是") {
        QMessageBox::information(this, "提示", "该报修单已处理");
        return;
    }

    // 获取报修编号
    QTableWidgetItem* orderIdItem = tableWidget->item(row, 0);
    if (!orderIdItem) return;

    QString orderId = orderIdItem->text();

    // 使用更灵活的对话框设置
    QInputDialog dialog(this);
    dialog.setWindowTitle("处理报修单");
    dialog.setLabelText("请输入处理详情:");
    dialog.setTextValue("");
    dialog.setOption(QInputDialog::UsePlainTextEditForTextInput);

    // 使用系统默认样式，避免手动设置大小
    dialog.setStyle(QStyleFactory::create("Fusion"));

    if (dialog.exec() == QDialog::Accepted) {
        QString processDesc = dialog.textValue();
        if (!processDesc.isEmpty()) {
            updateProcessStatus(orderId, processDesc);
        }
    }
}

void ProblemProcess::updateProcessStatus(const QString& orderId, const QString& processDesc)
{
    if (!m_dbManager->isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接");
        return;
    }

    // 开始事务
    if (!m_dbManager->beginTransaction()) {
        QMessageBox::critical(this, "错误", "无法开始数据库事务");
        return;
    }

    //尝试插入记录
    QString insertQuery = "INSERT INTO repair_progress "
                         "(order_id, process_status, process_desc, process_time, staff_id) "
                         "VALUES (?, ?, ?, ?, ?)";

    QVariantList insertValues;
    insertValues << orderId << true << processDesc << QDateTime::currentDateTime() << m_currentUserId;

    //执行插入
    bool insertSuccess = m_dbManager->executeQuery(insertQuery, insertValues);

    //如果插入失败，执行更新
    if (!insertSuccess) {
        QString updateQuery = "UPDATE repair_progress SET "
                             "process_status = ?, "
                             "process_desc = ?, "
                             "process_time = ?, "
                             "staff_id = ? "
                             "WHERE order_id = ?";

        QVariantList updateValues;
        updateValues << true << processDesc << QDateTime::currentDateTime() << m_currentUserId << orderId;

        if (!m_dbManager->executeQuery(updateQuery, updateValues)) {
            m_dbManager->rollbackTransaction();
            QMessageBox::critical(this, "错误", "更新处理状态失败");
            return;
        }
    }

    //提交事务
    if (!m_dbManager->commitTransaction()) {
        QMessageBox::critical(this, "错误", "无法提交数据库事务");
        return;
    }

    //刷新数据
    loadDataFromDatabase();

    QMessageBox::information(this, "成功", "报修单处理成功");
}

ProblemProcess::~ProblemProcess()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}
