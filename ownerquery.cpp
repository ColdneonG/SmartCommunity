#include "ownerquery.h"
#include "ui_ownerquery.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>

OwnerQuery::OwnerQuery(QWidget *parent)
    : QWidget(parent)
   , ui(new Ui::OwnerQuery)
   , dbManager(new DatabaseManager(this))
{
    // 生成唯一连接名（与OwnerModify保持一致）
    QString connName = QString("OwnerQuery_%1").arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    initUI();  // 初始化UI组件

    // 初始化数据库并加载数据
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", "初始化数据库失败：" + dbManager->lastError());
        return;
    }
}

OwnerQuery::~OwnerQuery()
{
    delete ui;
    // 数据库由dbManager析构时自动关闭（与OwnerModify一致）
}

void OwnerQuery::initUI()
{
    // 设置表格属性
    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setHorizontalHeaderLabels({
        "业主ID", "姓名", "身份证号", "电话", "邮箱", "入住日期", "注册时间"
    });
    // 表格设置为不可直接编辑（与OwnerModify保持一致）
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void OwnerQuery::on_searchButton_clicked()
{
    QString name = ui->nameEdit->text().trimmed();
    QString phone = ui->phoneEdit->text().trimmed();
    QString idNumber = ui->idNumberEdit->text().trimmed();

    QString sql = "SELECT owner_id, name, id_number, phone_number, email, move_in_date, register_time "
                  "FROM owner_info "
                  "WHERE 1=1 ";
    QVariantList bindValues;
    if (!name.isEmpty()) {
        sql += "AND name LIKE? ";
        bindValues << "%" + name + "%";
    }
    if (!phone.isEmpty()) {
        sql += "AND phone_number LIKE? ";
        bindValues << "%" + phone + "%";
    }
    if (!idNumber.isEmpty()) {
        sql += "AND id_number LIKE? ";
        bindValues << "%" + idNumber + "%";
    }

    QVector<QVariantMap> resultList = dbManager->executeSelectQuery(sql, bindValues);
    if (resultList.isEmpty() &&!dbManager->lastError().isEmpty()) {
        QMessageBox::critical(this, "查询错误", "查询执行失败：" + dbManager->lastError());
        return;
    }

    showQueryResult(resultList);
}

// 显示查询结果（与OwnerModify的showOwners方法风格统一）
void OwnerQuery::showQueryResult(const QVector<QVariantMap>& resultList)
{
    ui->tableWidget->setRowCount(0); // 清空表格
    for (const auto& rowData : resultList) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        // 按字段顺序填充表格（与OwnerModify的填充逻辑一致）
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(rowData["owner_id"].toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(rowData["name"].toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(rowData["id_number"].toString()));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(rowData["phone_number"].toString()));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(rowData["email"].toString()));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(rowData["move_in_date"].toString()));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(rowData["register_time"].toString()));
    }
}
