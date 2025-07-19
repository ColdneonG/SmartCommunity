#include "parkingrental.h"
#include "ui_parkingrental.h"
#include <QItemSelectionModel>
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include "databasemanager.h"

ParkingRental::ParkingRental(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingRental),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingRental_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {  // 初始化数据库
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }
    initTableWidget();
    refreshApplyData();
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParkingRental::onItemSelected);
}

ParkingRental::~ParkingRental()
{
    dbManager->closeDatabase();  // 关闭数据库连接
    delete ui;
}

void ParkingRental::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 30, 1300, 600);
    tableWidget->setColumnCount(6);
    QStringList headers;
    headers << "申请人" << "申请类型" << "是否需要充电功能" << "申请型号（大/小）" <<"申请时长（月）"<< "申请时间";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

//void ParkingRental::refreshApplyData()
//{
//    QString sql = "SELECT * FROM parking_apply where status=0 ORDER BY apply_time ASC";
//    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);  // 使用 DatabaseManager 执行查询
//    int rowCount = results.size();
//    tableWidget->setRowCount(rowCount);

//    rowToApplyIdMap.clear();
//    for (int row = 0; row < rowCount; ++row) {
//        QVariantMap result = results[row];
//        int applyId = result["apply_id"].toInt();
//        rowToApplyIdMap[row] = applyId;

//        QTableWidgetItem* item1 = new QTableWidgetItem(result["type"].toString());
//        item1->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 0, item1);

//        QTableWidgetItem* item2 = new QTableWidgetItem(result["apply_type"].toString());
//        item2->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 1, item2);

//        QString chargableText = result["is_chargable"].toInt() == 1 ? "是" : "否";
//        QTableWidgetItem* item3 = new QTableWidgetItem(chargableText);
//        item3->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 2, item3);

//        QTableWidgetItem* item4 = new QTableWidgetItem(result["apply_size"].toString());
//        item4->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 3, item4);

//        QTableWidgetItem* item5 = new QTableWidgetItem(result["apply_length"].toString());
//        item5->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 4, item5);

//        QTableWidgetItem* item6 = new QTableWidgetItem(result["apply_time"].toString());
//        item6->setTextAlignment(Qt::AlignCenter);
//        tableWidget->setItem(row, 5, item6);
//    }
//}
void ParkingRental::refreshApplyData()
{
    QString sql = "SELECT * FROM parking_apply where status=0 ORDER BY apply_time ASC";
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    int rowCount = results.size();
    tableWidget->setRowCount(rowCount);

    rowToApplyIdMap.clear();
    for (int row = 0; row < rowCount; ++row) {
        QVariantMap result = results[row];
        int applyId = result["apply_id"].toInt();
        rowToApplyIdMap[row] = applyId;

        // 对应表中 type 字段
        QTableWidgetItem* item1 = new QTableWidgetItem(result["owner_id"].toString());
        item1->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, item1);

        // 表中无 apply_type ，这里逻辑有误，假设想展示和申请类型相关，若表设计就是 type 代表类型，可直接用，若需其他，需确认表结构补充字段
        // 这里先延续用 type 演示，若实际有其他含义，需按真实逻辑调整
        QTableWidgetItem* item2 = new QTableWidgetItem(result["type"].toString());
        item2->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, item2);

        QString chargableText = result["is_chargeable"].toInt() == 1 ? "是" : "否";
        QTableWidgetItem* item3 = new QTableWidgetItem(chargableText);
        item3->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 2, item3);

        // 对应表中 size 字段
        QTableWidgetItem* item4 = new QTableWidgetItem(result["size"].toString());
        item4->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 3, item4);

        // 对应表中 length 字段
        QTableWidgetItem* item5 = new QTableWidgetItem(result["length"].toString());
        item5->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 4, item5);

        // 对应表中 apply_time 字段
        QTableWidgetItem* item6 = new QTableWidgetItem(result["apply_time"].toString());
        item6->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 5, item6);
    }
}

void ParkingRental::onItemSelected(){
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        applyId = -1;
        return;
    }

    int row = selectedRows.first().row();
    applyId = rowToApplyIdMap.contains(row) ? rowToApplyIdMap[row] : -1;
}

void ParkingRental::on_pushButton_rental_clicked()
{
    if(applyId==-1){
        QMessageBox::critical(this,"错误","请先选择一项数据");
        return;
    }

    QString sql=QString("select * from parking_apply where apply_id=%1").arg(applyId);
qDebug()<<sql;
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到申请记录");
        return;
    }
    QVariantMap result = results.first();
    QString applyType = result["type"].toString();
    int applyIsCharagable = result["is_chargeable"].toInt();
    QString applySize = result["size"].toString();
    QString applyLength = result["length"].toString();

    sql = QString("select count(*) from parking_space where type='%1' and is_chargeable=%2 and size='%3' and status=0")
          .arg(applyType)
          .arg(applyIsCharagable)
          .arg(applySize);
qDebug()<<sql;
    results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "查询空闲车位数量失败");
        return;
    }
    int cnt = results.first()["count(*)"].toInt();
qDebug()<<cnt;
    if(cnt==0){
        QMessageBox::critical(this,"分配失败","目前没有符合条件的空闲车位，无法分配！");
    }
    else{
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认分配",
                                      QString("目前有 %1 个符合条件的空闲车位，是否确定分配？").arg(cnt),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;

        sql = QString("select * from parking_space where type='%1' and is_chargeable=%2 and size='%3' and status=0")
                .arg(applyType)
              .arg(applyIsCharagable)
              .arg(applySize);
        results = dbManager->executeSelectQuery(sql);
        if (results.isEmpty()) {
            QMessageBox::critical(this, "错误", "未找到符合条件的空闲车位");
            return;
        }
        result = results.first();
        int spaceId = result["space_id"].toInt();
        QString spaceName = result["location"].toString();

        sql = QString("select * from parking_apply where apply_id=%1").arg(applyId);
        results = dbManager->executeSelectQuery(sql);
        if (results.isEmpty()) {
            QMessageBox::critical(this, "错误", "未找到申请记录");
            return;
        }
        result = results.first();
        int ownerId = result["owner_id"].toInt();
        QString plate = result["plate"].toString();

        QDateTime currentTime = QDateTime::currentDateTime();
        int monthsToAdd = applyLength.toInt();
        QDateTime endTime = currentTime.addMonths(monthsToAdd);
        QString getFeeSql = QString("select fee from item_fee where item_id=1");
        QVector<QVariantMap> feeResults = dbManager->executeSelectQuery(getFeeSql);
        if (feeResults.isEmpty()) {
            QMessageBox::critical(this, "错误", "未找到 item_fee 中 id 为 1 的记录");
            return;
        }
        double unitPrice = feeResults.first()["fee"].toDouble();
        double fee = unitPrice * monthsToAdd;

        dbManager->beginTransaction();  // 开始事务
        sql = QString("update  parking_space set status=1 where space_id=%1").arg(spaceId);
        if (!dbManager->executeQuery(sql)) {
            dbManager->rollbackTransaction();  // 回滚事务
            QMessageBox::critical(this, "错误", "更新车位状态失败: " + dbManager->lastError());
            return;
        }

        sql = QString("insert into parking_rental (space_id,owner_id,start_time,end_time,rent_fee,plate)"
                      "values(%1,%2,'%3','%4',%5,'%6')")
              .arg(spaceId)
              .arg(ownerId)
              .arg(currentTime.toString("yyyy-MM-dd hh:mm:ss"))
              .arg(endTime.toString("yyyy-MM-dd hh:mm:ss"))
              .arg(fee)
              .arg(plate);
        if (!dbManager->executeQuery(sql)) {
            dbManager->rollbackTransaction();  // 回滚事务
            QMessageBox::critical(this, "错误", "插入租赁记录失败: " + dbManager->lastError());
            return;
        }

        sql = QString("update parking_apply set status=1 where apply_id=%1").arg(applyId);
        if (!dbManager->executeQuery(sql)) {
            dbManager->rollbackTransaction();  // 回滚事务
            QMessageBox::critical(this, "错误", "更新申请状态失败: " + dbManager->lastError());
            return;
        }

        QString insertFeeLogSql = QString("insert into fee_log (owner_id, fee_type, value, payment_status, time)"
                                          "values(%1, 1, %2, 0, '%3')")
                                  .arg(ownerId)
                                  .arg(fee)
                                  .arg(currentTime.toString("yyyy-MM-dd hh:mm:ss"));
        if (!dbManager->executeQuery(insertFeeLogSql)) {
            dbManager->rollbackTransaction();  // 回滚事务
            QMessageBox::critical(this, "错误", "插入 fee_log 记录失败: " + dbManager->lastError());
            return;
        }

        if (!dbManager->commitTransaction()) {  // 提交事务
            QMessageBox::critical(this, "错误", "提交事务失败: " + dbManager->lastError());
            return;
        }

        QMessageBox::information(this,"成功",QString("已经将车位%1分配给%2").arg(spaceName).arg(ownerId));
        refreshApplyData();
    }
}
