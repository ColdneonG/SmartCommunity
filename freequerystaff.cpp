#include "freequerystaff.h"
#include "ui_freequerystaff.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include "databasemanager.h"

FreeQueryStaff::FreeQueryStaff(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FreeQueryStaff),
    m_dbManager(new DatabaseManager(this)),
    tableWidget(nullptr),
    selectedFeeId(-1)
{
    // 生成唯一连接名（与其他类保持一致）
    QString connName = QString("FreeQueryStaff_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    initUI(); // 集中初始化UI组件

    // 数据库初始化（完全依赖DatabaseManager）
    if (!m_dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", "初始化数据库失败：" + m_dbManager->lastError());
        return;
    }

    // 初始化完成后加载数据
    refreshData();
}

FreeQueryStaff::~FreeQueryStaff()
{
    delete ui;
    // 数据库连接由DatabaseManager析构时自动关闭
}

// 集中初始化UI组件（包括表格和日期控件）
void FreeQueryStaff::initUI()
{
    // 初始化表格
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 290, 1300, 600);
    tableWidget->setColumnCount(5);
    QStringList headers;
    headers << "ID" << "费用类型" << "缴费状态" << "时间" << "金额";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 绑定选择事件
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FreeQueryStaff::onItemSelected);

    // 初始化日期控件
    ui->dateEdit_start->setDate(QDate(2020, 1, 1));
    ui->dateEdit_end->setDate(QDate::currentDate());
    connect(ui->dateEdit_start, &QDateEdit::dateChanged,
            this, &FreeQueryStaff::onStartDateChanged);
}

void FreeQueryStaff::onStartDateChanged(const QDate &date)
{
    ui->dateEdit_end->setMinimumDate(date);
    if (ui->dateEdit_end->date() < date) {
        ui->dateEdit_end->setDate(date);
    }
}

void FreeQueryStaff::on_pushButton_query_clicked()
{
    refreshData();
}

void FreeQueryStaff::refreshData()
{
    // 获取用户输入的筛选条件
    QString id = ui->le_id->text().trimmed();
    QString typeText = ui->comboBox_type_2->currentText();
    QString statusText = ui->comboBox_status_2->currentText();
    QDate startDate = ui->dateEdit_start->date();
    QDate endDate = ui->dateEdit_end->date();

    // 构建查询条件（使用参数绑定防止SQL注入）
    QString sql = "SELECT * FROM fee_log WHERE 1=1 ";
    QVariantList bindValues;

    if (!id.isEmpty()) {
        sql += "AND owner_id LIKE? ";
        bindValues << "%" + id + "%";
    }

    if (!typeText.isEmpty() && typeText != "empty" && typeText != "全部") {
        int typeValue = 0;
        if (typeText == "水费") typeValue = 2;
        else if (typeText == "物业费") typeValue = 3;
        else if (typeText == "车位费") typeValue = 1;

        sql += "AND fee_type =? ";
        bindValues << typeValue;
    }

    if (!statusText.isEmpty() && statusText != "empty" && statusText != "全部") {
        int statusValue = (statusText == "已缴费") ? 1 : 0;
        sql += "AND payment_status =? ";
        bindValues << statusValue;
    }

    if (startDate.isValid() && endDate.isValid()) {
        QString startStr = startDate.toString("yyyy-MM-dd 00:00:00");
        QString endStr = endDate.toString("yyyy-MM-dd 23:59:59");
        sql += "AND time BETWEEN? AND? ";
        bindValues << startStr << endStr;
    }

    sql += "ORDER BY owner_id DESC";

    // 使用DatabaseManager执行查询（带参数绑定）
    QVector<QVariantMap> resultList = m_dbManager->executeSelectQuery(sql, bindValues);
    if (resultList.isEmpty() &&!m_dbManager->lastError().isEmpty()) {
        QMessageBox::warning(this, "查询失败", "加载数据失败：" + m_dbManager->lastError());
        return;
    }

    // 显示查询结果
    showFeeLogs(resultList);
}

// 显示费用记录到表格（与其他类的显示逻辑保持一致）
void FreeQueryStaff::showFeeLogs(const QVector<QVariantMap>& feeLogs)
{
    tableWidget->setRowCount(0); // 清空表格
    rowToFeeIdMap.clear(); // 清空映射

    int row = 0;
    for (const auto& log : feeLogs) {
        tableWidget->insertRow(row);
        rowToFeeIdMap[row] = log["fee_id"].toInt();

        // 填充表格数据（逐项设置，与其他类保持一致）
        tableWidget->setItem(row, 0, new QTableWidgetItem(log["owner_id"].toString()));

        QString typeStr;
        int type = log["fee_type"].toInt();
        if (type == 2) typeStr = "水费";
        else if (type == 3) typeStr = "物业费";
        else if (type == 1) typeStr = "车位费";
        tableWidget->setItem(row, 1, new QTableWidgetItem(typeStr));

        QString statusStr = (log["payment_status"].toInt() == 1) ? "已缴费" : "未缴费";
        tableWidget->setItem(row, 2, new QTableWidgetItem(statusStr));

        tableWidget->setItem(row, 3, new QTableWidgetItem(log["time"].toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(log["value"].toString()));

        // 设置单元格居中对齐
        for (int col = 0; col < 5; ++col) {
            tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
        }

        row++;
    }
}

void FreeQueryStaff::onItemSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    QModelIndexList selectedRows = selected.indexes();
    if (selectedRows.isEmpty()) {
        selectedFeeId = -1;
        return;
    }

    int row = selectedRows.first().row();
    selectedFeeId = rowToFeeIdMap.value(row, -1);
}

void FreeQueryStaff::on_pushButton_query_2_clicked()
{
    if (selectedFeeId == -1) {
        QMessageBox::warning(this, "提示", "请先选择一条数据");
        return;
    }

    // 查询当前选中记录的缴费状态（通过DatabaseManager）
    QString sql = "SELECT payment_status FROM fee_log WHERE fee_id =? ";
    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(sql, {selectedFeeId});
    if (result.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到选中的记录");
        return;
    }

    int status = result.first()["payment_status"].toInt();
    if (status == 1) {
        QMessageBox::information(this, "提示", "该记录已缴费");
        return;
    }

    if (QMessageBox::question(this, "确认缴费", "确定要将此记录标记为已缴费吗？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // 执行更新（使用参数绑定）
        QString updateSql = "UPDATE fee_log SET payment_status = 1 WHERE fee_id =? ";
        if (m_dbManager->executeQuery(updateSql, {selectedFeeId})) {
            QMessageBox::information(this, "成功", "缴费状态更新成功！");
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "更新失败：" + m_dbManager->lastError());
        }
    }
}
