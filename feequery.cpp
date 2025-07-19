#include "feequery.h"
#include "ui_feequery.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>
#include "databasemanager.h"

FeeQuery::FeeQuery(int owner_id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeeQuery),
    owner_id(owner_id),
    m_dbManager(new DatabaseManager(this))
{
    ui->setupUi(this);
    if (!initDatabaseConnection()) {
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }
    initTableWidget();
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FeeQuery::onItemSelected);
    initDateEdit();
    refreshData();
}

FeeQuery::~FeeQuery()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}

bool FeeQuery::initDatabaseConnection()
{
    if (m_dbManager->isOpen()) {
        return true;
    }

    QString connName = QString("FeeQuery_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    return m_dbManager->initDatabase(connName);
}

// 初始化表格（移除第一列ID，保留4列）
void FeeQuery::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 290, 1300, 600);
    tableWidget->setColumnCount(4);  // 列数改为4（移除ID列）
    QStringList headers;
    headers << "费用类型" << "缴费状态" << "时间" << "金额";  // 表头去掉ID
    tableWidget->setHorizontalHeaderLabels(headers);

    // 表格样式设置（与原逻辑一致）
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

// 初始化日期选择器（与原逻辑一致）
void FeeQuery::initDateEdit()
{
    ui->dateEdit_start->setDate(QDate(2020, 1, 1));
    ui->dateEdit_end->setDate(QDate::currentDate());
    connect(ui->dateEdit_start, &QDateEdit::dateChanged, this, &FeeQuery::onStartDateChanged);
}

// 刷新数据（移除第一列数据填充，调整列索引）
void FeeQuery::refreshData()
{
    // 获取筛选条件（保留类型、状态、日期范围）
    QString typeText = ui->comboBox_type_2->currentText();
    QString statusText = ui->comboBox_status_2->currentText();
    QDate startDate = ui->dateEdit_start->date();
    QDate endDate = ui->dateEdit_end->date();

    // 构建查询条件（强制筛选当前owner_id）
    QList<QString> conditions;
    conditions.append(QString("owner_id = %1").arg(owner_id));  // 固定ID筛选

    // 费用类型条件
    if (!typeText.isEmpty() && typeText != "empty" && typeText != "全部") {
        int typeValue = 0;
        if (typeText == "水费") typeValue = 2;
        else if (typeText == "物业费") typeValue = 3;
        else if (typeText == "车位费") typeValue = 1;
        conditions.append(QString("fee_type = %1").arg(typeValue));
    }

    // 缴费状态条件
    if (!statusText.isEmpty() && statusText != "empty" && statusText != "全部") {
        int statusValue = (statusText == "已缴费") ? 1 : 0;
        conditions.append(QString("payment_status = %1").arg(statusValue));
    }

    // 日期范围条件
    if (startDate.isValid() && endDate.isValid()) {
        QString startStr = startDate.toString("yyyy-MM-dd 00:00:00");
        QString endStr = endDate.toString("yyyy-MM-dd 23:59:59");
        conditions.append(QString("time BETWEEN '%1' AND '%2'").arg(startStr, endStr));
    }

    // 构建SQL查询
    QString sql = "SELECT * FROM fee_log";
    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }
    sql += " ORDER BY owner_id DESC";

    // 调试输出SQL
    qDebug() << "执行的SQL语句:" << sql;

    // 执行查询
    QVector<QVariantMap> resultList = m_dbManager->executeSelectQuery(sql, {});

    tableWidget->setRowCount(0);
    int row = 0;
    for (const auto& rowData : resultList) {
        tableWidget->insertRow(row);
        rowToFeeIdMap[row]=rowData["fee_id"].toInt();
        QString typeStr;
        int typeValue = rowData["fee_type"].toInt();
        if (typeValue == 2) typeStr = "水费";
        else if (typeValue == 3) typeStr = "物业费";
        else if (typeValue == 1) typeStr = "车位费";
        QTableWidgetItem* typeItem = new QTableWidgetItem(typeStr);
        typeItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, typeItem);

        QString statusStr = (rowData["payment_status"].toInt() == 1) ? "已缴费" : "未缴费";
        QTableWidgetItem* statusItem = new QTableWidgetItem(statusStr);
        statusItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, statusItem);

        QTableWidgetItem* timeItem = new QTableWidgetItem(rowData["time"].toString());
        timeItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 2, timeItem);

        QTableWidgetItem* valueItem = new QTableWidgetItem(rowData["value"].toString());
        valueItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 3, valueItem);

        row++;
    }

    qDebug() << "查询到" << row << "条记录";
}

void FeeQuery::onStartDateChanged(const QDate &date)
{
    ui->dateEdit_end->setMinimumDate(date);
    if (ui->dateEdit_end->date() < date) {
        ui->dateEdit_end->setDate(date);
    }
}

void FeeQuery::on_pushButton_query_clicked()
{
    refreshData();
}

void FeeQuery::on_pushButton_query_2_clicked()
{
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();

    // 检查是否选择了数据
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一条数据");
        return;
    }

    // 获取选中行的ID和缴费状态
    int selectedRow = selectedRows.first().row();
    QString id = tableWidget->item(selectedRow, 0)->text();
    QString status = tableWidget->item(selectedRow, 1)->text(); // 修正索引为1

    // 检查是否已经缴费
    if (status == "已缴费") {
        QMessageBox::information(this, "提示", "该记录已缴费");
        return;
    }

    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认缴费", "确定要将此记录标记为已缴费吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 构建SQL更新语句
        QString sql = QString("UPDATE fee_log SET payment_status = 1 WHERE fee_id=%1").arg(selectedFeeId);
        qDebug() << sql;
        m_dbManager->executeQuery(sql, {});

        QMessageBox::information(this, "成功", "缴费状态更新成功！");
        refreshData(); // 刷新数据
    }
}

void FeeQuery::onItemSelected()
{
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        selectedFeeId = -1;
        return;
    }

    int row = selectedRows.first().row();
    selectedFeeId = rowToFeeIdMap.contains(row) ? rowToFeeIdMap[row] : -1;
}
