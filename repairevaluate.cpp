#include "repairevaluate.h"
#include "ui_repairevaluate.h"
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>

RepairEvaluate::RepairEvaluate(int ownerId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RepairEvaluate)
    , m_dbManager(new DatabaseManager(this))
    , tableWidget(nullptr)
    , m_currentOwnerId(ownerId)
{
    QString connName = QString("RepairEvaluate_%1")
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

    // 确保按钮在最上层
    ui->refreshButton->raise();
    ui->evaluateButton->raise();
}

void RepairEvaluate::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    // 使用布局管理器代替固定大小和位置
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableWidget);
    mainLayout->setContentsMargins(30, 290, 30, 30); // 设置边距

    tableWidget->setColumnCount(6);
    QStringList headers;
    headers << "报修编号" << "报修时间" << "故障描述" << "是否已处理" << "处理详情" << "处理时间";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置选择模型，确保选择行为正确
    tableWidget->setSelectionModel(new QItemSelectionModel(tableWidget->model(), this));
}

void RepairEvaluate::loadDataFromDatabase()
{
    tableWidget->clearContents();
    tableWidget->setRowCount(0);

    if (!m_dbManager->isOpen()) {
        qDebug() << "数据库未打开";
        return;
    }

    // 根据ownerId查询order_id
    QString orderIdQuery = "SELECT order_id FROM repair_order WHERE owner_id = ?";
    QVector<QVariantMap> orderIdResult = m_dbManager->executeSelectQuery(orderIdQuery, {m_currentOwnerId});

    if (orderIdResult.isEmpty()) {
        qDebug() << "未找到该业主的报修记录";
        QMessageBox::information(this, "提示", "未找到您的报修记录");
        return;
    }

    // 提取所有order_id
    QStringList orderIds;
    for (const auto& row : orderIdResult) {
        if (row.contains("order_id")) {
            orderIds.append(row["order_id"].toString());
        }
    }

    if (orderIds.isEmpty()) {
        qDebug() << "提取order_id失败";
        QMessageBox::information(this, "提示", "未找到有效的报修单");
        return;
    }

    // 使用提取的order_id查询报修单详情，只显示已处理的订单
    QString queryStr = QString("SELECT "
                             "ro.order_id, "
                             "ro.order_time, "
                             "ro.problem_desc, "
                             "rp.process_status, "
                             "rp.process_desc, "
                             "rp.process_time, "
                             "rp.staff_id "
                             "FROM repair_order ro "
                             "LEFT JOIN repair_progress rp ON ro.order_id = rp.order_id "
                             "WHERE ro.order_id IN (");

    // 生成占位符列表
    QStringList placeholders;
    for (int i = 0; i < orderIds.size(); ++i) {
        placeholders.append("?");
    }
    queryStr += placeholders.join(",") + ") AND rp.process_status = ?";

    // 准备绑定参数列表
    QVariantList bindValues;
    for (const QString& orderId : orderIds) {
        bindValues.append(orderId);
    }
    bindValues.append(1); // 添加process_status参数

    // 执行查询
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr, bindValues);

    if (result.isEmpty()) {
        qDebug() << "未找到已处理的报修记录";
        QMessageBox::information(this, "提示", "没有已处理的报修单可供评价");
        return;
    }

    for (const auto& row : result) {
        if (!row.contains("order_id")) {
            qDebug() << "跳过无效的记录，缺少order_id字段";
            continue;
        }

        int rowIndex = tableWidget->rowCount();
        tableWidget->insertRow(rowIndex);

        // 填充报修编号
        tableWidget->setItem(rowIndex, 0, new QTableWidgetItem(row["order_id"].toString()));

        // 填充报修时间
        QDateTime orderTime = row["order_time"].toDateTime();
        tableWidget->setItem(rowIndex, 1, new QTableWidgetItem(orderTime.toString("yyyy-MM-dd hh:mm:ss")));

        // 填充故障描述
        tableWidget->setItem(rowIndex, 2, new QTableWidgetItem(row["problem_desc"].toString()));

        // 填充是否已处理
        bool isProcessed = row["process_status"].toBool();
        tableWidget->setItem(rowIndex, 3, new QTableWidgetItem(getStatusText(isProcessed)));

        // 填充处理详情
        tableWidget->setItem(rowIndex, 4, new QTableWidgetItem(row["process_desc"].toString()));

        // 填充处理时间
        QDateTime processTime = row["process_time"].toDateTime();
        tableWidget->setItem(rowIndex, 5, new QTableWidgetItem(processTime.isValid() ?
                                                         processTime.toString("yyyy-MM-dd hh:mm:ss") : ""));

        // 存储处理人ID，用于后续评价
        m_orderStaffMap[row["order_id"].toString()] = row["staff_id"].toString();
    }

    qDebug() << "数据加载完成，共" << tableWidget->rowCount() << "条记录";
}

QString RepairEvaluate::getStatusText(bool isProcessed) const
{
    return isProcessed ? "是" : "否";
}

void RepairEvaluate::on_refreshButton_clicked()
{
    loadDataFromDatabase();
}

void RepairEvaluate::on_evaluateButton_clicked()
{
    // 获取选中行
    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要评价的报修单");
        return;
    }

    // 获取选中行的行号
    int row = selectedItems.first()->row();

    // 获取报修编号
    QTableWidgetItem* orderIdItem = tableWidget->item(row, 0);
    if (!orderIdItem) {
        QMessageBox::information(this, "提示", "无效的报修单选择");
        return;
    }

    QString orderId = orderIdItem->text();

    // 检查该订单是否已经评价过
    if (hasEvaluated(orderId)) {
        QMessageBox::information(this, "提示", "该报修单已评价，不能重复评价");
        return;
    }

    // 获取对应的处理人ID
    QString staffId = m_orderStaffMap.value(orderId);
    if (staffId.isEmpty()) {
        QMessageBox::critical(this, "错误", "无法获取处理人员信息");
        return;
    }

    // 创建评价对话框
    QDialog dialog(this);
    dialog.setWindowTitle("评价报修单");
    dialog.resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // 添加评分选择
    QLabel *scoreLabel = new QLabel("评分 (1-10分):", &dialog);
    QSpinBox *scoreSpinBox = new QSpinBox(&dialog);
    scoreSpinBox->setRange(1, 10);

    // 添加评价内容输入
    QLabel *contentLabel = new QLabel("评价内容:", &dialog);
    QTextEdit *contentTextEdit = new QTextEdit(&dialog);

    // 添加按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addWidget(scoreLabel);
    layout->addWidget(scoreSpinBox);
    layout->addWidget(contentLabel);
    layout->addWidget(contentTextEdit);
    layout->addWidget(buttonBox);

    // 连接按钮信号
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        int score = scoreSpinBox->value();
        QString content = contentTextEdit->toPlainText();

        if (content.isEmpty()) {
            QMessageBox::information(this, "提示", "评价内容不能为空");
            return;
        }

        // 保存评价
        saveEvaluation(orderId, score, content, staffId);
    }
}

bool RepairEvaluate::hasEvaluated(const QString &orderId)
{
    if (!m_dbManager->isOpen()) {
        qDebug() << "数据库未打开";
        return false;
    }

    QString queryStr = "SELECT COUNT(*) FROM repair_evaluation WHERE order_id = ?";
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr, {orderId});

    if (!result.isEmpty() && result.first().contains("COUNT(*)")) {
        return result.first()["COUNT(*)"].toInt() > 0;
    }

    return false;
}

void RepairEvaluate::saveEvaluation(const QString &orderId, int score, const QString &content, const QString &staffId)
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

    // 插入评价记录
    QString insertQuery = "INSERT INTO repair_evaluation "
                         "(order_id, owner_id, staff_id, score, eval_content, eval_time) "
                         "VALUES (?, ?, ?, ?, ?, ?)";

    QVariantList insertValues;
    insertValues << orderId << m_currentOwnerId << staffId << score << content << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 执行插入
    if (!m_dbManager->executeQuery(insertQuery, insertValues)) {
        m_dbManager->rollbackTransaction();
        QMessageBox::critical(this, "错误", "提交评价失败");
        return;
    }

    // 提交事务
    if (!m_dbManager->commitTransaction()) {
        QMessageBox::critical(this, "错误", "无法提交数据库事务");
        return;
    }

    // 刷新数据
    loadDataFromDatabase();

    QMessageBox::information(this, "成功", "评价提交成功");
}

RepairEvaluate::~RepairEvaluate()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}
