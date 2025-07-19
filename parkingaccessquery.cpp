#include "parkingaccessquery.h"
#include "ui_parkingaccessquery.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include "databasemanager.h"
#include <QMessageBox>

ParkingAccessQuery::ParkingAccessQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingAccessQuery),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingAccessQuery_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }
    initTableWidget();
    initDateEdit();
    refreshData();
}

ParkingAccessQuery::~ParkingAccessQuery()
{
    dbManager->closeDatabase();
    delete ui;
}

void ParkingAccessQuery::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 290, 1300, 600);
    tableWidget->setColumnCount(3);
    QStringList headers;
    headers << "车牌号" << "类型" << "时间";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void ParkingAccessQuery::initDateEdit(){
    ui->dateEdit_start->setDate(QDate(2020, 1, 1));
    ui->dateEdit_end->setDate(QDate::currentDate());
    connect(ui->dateEdit_start, &QDateEdit::dateChanged, this, &ParkingAccessQuery::onStartDateChanged);
}

void ParkingAccessQuery::refreshData()
{
    // 获取用户输入的筛选条件
    QString plate = ui->le_plate->text().trimmed();
    QString typeText = ui->comboBox_type->currentText();
    QDate startDate = ui->dateEdit_start->date();
    QDate endDate = ui->dateEdit_end->date();

    // 构建查询条件列表
    QString whereClause = "WHERE 1=1"; // 基础条件，始终为真
    QList<QString> conditions;

    // 添加车牌号条件
    if (!plate.isEmpty()) {
        conditions.append(QString("plate LIKE '%%1%'").arg(plate));
    }

    // 添加类型条件
    if (!typeText.isEmpty() && typeText != "empty") {
        int typeValue = (typeText == "进入") ? 1 : 0;
        conditions.append(QString("type = %1").arg(typeValue));
    }

    // 添加时间范围条件
    if (startDate.isValid() && endDate.isValid()) {
        // 开始日期 >= 指定日期的00:00:00
        QString startStr = startDate.toString("yyyy-MM-dd 00:00:00");
        // 结束日期 <= 指定日期的23:59:59
        QString endStr = endDate.toString("yyyy-MM-dd 23:59:59");
        conditions.append(QString("time BETWEEN '%1' AND '%2'").arg(startStr, endStr));
    }

    // 组合所有条件
    if (!conditions.isEmpty()) {
        whereClause += " AND " + conditions.join(" AND ");
    }

    // 构建完整SQL查询
    QString sql = QString("SELECT plate, type, time FROM vehicle_access_log %1 ORDER BY time DESC").arg(whereClause);

    // 执行查询
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);

    // 清空表格并准备填充数据
    tableWidget->setRowCount(0);

    for (int row = 0; row < results.size(); ++row) {
        QVariantMap result = results[row];
        tableWidget->insertRow(row);

        // 设置车牌号
        QTableWidgetItem* plateItem = new QTableWidgetItem(result["plate"].toString());
        plateItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, plateItem);

        // 设置类型（转换为中文显示）
        QString typeStr = result["type"].toInt() == 1 ? "进入" : "离开";
        QTableWidgetItem* typeItem = new QTableWidgetItem(typeStr);
        typeItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, typeItem);

        // 设置时间
        QTableWidgetItem* timeItem = new QTableWidgetItem(result["time"].toString());
        timeItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 2, timeItem);
    }
}

void ParkingAccessQuery::onStartDateChanged(const QDate &date)
{
    ui->dateEdit_end->setMinimumDate(date);

    if (ui->dateEdit_end->date() < date) {
        ui->dateEdit_end->setDate(date);
    }
}

void ParkingAccessQuery::on_pushButton_query_clicked()
{
    refreshData();
}
