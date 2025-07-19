#include "parkingrentalmanage.h"
#include "ui_parkingrentalmanage.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog>
#include "databasemanager.h"

ParkingRentalManage::ParkingRentalManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParkingRentalManage),
    dbManager(new DatabaseManager(this))
{
    QString connName = QString("ParkingRentalManage_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "错误", "数据库初始化失败: " + dbManager->lastError());
        return;
    }
    initTableWidget();
    initConnections();
    refreshData();
}

ParkingRentalManage::~ParkingRentalManage()
{
    dbManager->closeDatabase();
    delete ui;
}

void ParkingRentalManage::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 290, 1300, 600);
    tableWidget->setColumnCount(5);
    QStringList headers;
    headers <<"车位编号"<< "车牌号" << "车位到期时间" <<"待缴费用"<<"缴费状态";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void ParkingRentalManage::initConnections(){
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParkingRentalManage::onItemSelected);
}

void ParkingRentalManage::onItemSelected()
{
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        selectedRentalId = -1;
        return;
    }

    int row = selectedRows.first().row();
    selectedRentalId = rowToRentalIdMap.contains(row) ? rowToRentalIdMap[row] : -1;
}

void ParkingRentalManage::refreshData(){
    QString sql = QString("select * from parking_rental where payment_status!=3");
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    int rowCount = results.size();
    tableWidget->setRowCount(rowCount);

    rowToRentalIdMap.clear();
    for (int row = 0; row < rowCount; ++row) {
        QVariantMap result = results[row];
        int rentalId = result["rental_id"].toInt();
        rowToRentalIdMap[row] = rentalId;

        QTableWidgetItem* item1 = new QTableWidgetItem(result["space_id"].toString());
        item1->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, item1);

        QTableWidgetItem* item2 = new QTableWidgetItem(result["plate"].toString());
        item2->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, item2);

        QDateTime expireTime = result["end_time"].toDateTime();
        QTableWidgetItem* item3 = new QTableWidgetItem(expireTime.toString("yyyy-MM-dd hh:mm:ss"));
        item3->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 2, item3);

        double fee = result["rent_fee"].toDouble();
        QTableWidgetItem* item4 = new QTableWidgetItem(QString::number(fee, 'f', 2));
        item4->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 3, item4);

        int paymentStatus = result["payment_status"].toInt();
        QTableWidgetItem* item5 = new QTableWidgetItem(paymentStatus == 1 ? "已缴费" : "未缴费");
        item5->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 4, item5);
    }
}

void ParkingRentalManage::on_pushButton_delete_clicked()
{
    if (selectedRentalId == -1) {
        QMessageBox::warning(this, "警告", "请先选择一条记录");
        return;
    }

    dbManager->beginTransaction();
    QString sql = QString("update parking_rental set payment_status =3 where rental_id=%1").arg(selectedRentalId);
    if (!dbManager->executeQuery(sql)) {
        dbManager->rollbackTransaction();
        QMessageBox::critical(this, "错误", "更新租赁记录状态失败: " + dbManager->lastError());
        return;
    }

    sql = QString("select * from parking_rental where rental_id=%1").arg(selectedRentalId);
    QVector<QVariantMap> results = dbManager->executeSelectQuery(sql);
    if (results.isEmpty()) {
        dbManager->rollbackTransaction();
        QMessageBox::critical(this, "错误", "未找到租赁记录");
        return;
    }
    QVariantMap result = results.first();
    int space_id = result["space_id"].toInt();
    int owner_id = result["owner_id"].toInt();

    sql = QString("update parking_space set status=0 where space_id= %1").arg(space_id);
    if (!dbManager->executeQuery(sql)) {
        dbManager->rollbackTransaction();
        QMessageBox::critical(this, "错误", "更新车位状态失败: " + dbManager->lastError());
        return;
    }

    sql = QString("update parking_apply set status=3 where owner_id=%1").arg(owner_id);
    if (!dbManager->executeQuery(sql)) {
        dbManager->rollbackTransaction();
        QMessageBox::critical(this, "错误", "更新申请状态失败: " + dbManager->lastError());
        return;
    }

    if (!dbManager->commitTransaction()) {
        QMessageBox::critical(this, "错误", "提交事务失败: " + dbManager->lastError());
        return;
    }

    QMessageBox::information(this,"提示","解除出租关系成功");
    refreshData();
}

void ParkingRentalManage::on_pushButton_continue_clicked()
{
    if (selectedRentalId == -1) {
        QMessageBox::warning(this, "警告", "请先选择一条记录");
        return;
    }

    QString selectSql = QString("SELECT end_time FROM parking_rental WHERE rental_id = %1").arg(selectedRentalId);
    QVector<QVariantMap> results = dbManager->executeSelectQuery(selectSql);
    if (results.isEmpty()) {
        QMessageBox::critical(this, "错误", "未找到租赁记录");
        return;
    }
    QVariantMap result = results.first();
    QDateTime currentEndTime = result["end_time"].toDateTime();

    bool ok;
    QString input = QInputDialog::getText(this,"续约","请输入续约时长（月）",
                                          QLineEdit::Normal,"",&ok);
    if(!ok) return;

    QRegularExpression regex("^[1-9]\\d*$");
    if (!regex.match(input).hasMatch()) {
        QMessageBox::critical(this, "输入错误", "续约时长必须是正整数！");
        return;
    }
    int months = input.toInt();
    QDateTime newEndTime = currentEndTime.addMonths(months);

    QString updateSql = QString(
        "UPDATE parking_rental "
        "SET end_time = '%1' "
        "WHERE rental_id = %2"
    ).arg(newEndTime.toString("yyyy-MM-dd hh:mm:ss"))
      .arg(selectedRentalId);

    if (!dbManager->executeQuery(updateSql)) {
        QMessageBox::critical(this, "错误", "更新租赁记录到期时间失败: " + dbManager->lastError());
        return;
    }

    QMessageBox::information(this, "成功", QString("续约成功！新的到期时间为：%1").arg(newEndTime.toString("yyyy-MM-dd hh:mm:ss")));
    refreshData();
}
