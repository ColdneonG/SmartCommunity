#include "feemanage.h"
#include "ui_feemanage.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include "databasemanager.h"

FeeManage::FeeManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeeManage),
    m_dbManager(new DatabaseManager(this))
{
    ui->setupUi(this);
    if (!initDatabaseConnection()) {
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }
    initTableWidget();
    initButtons();
    initConnections();
    refreshData();
}

FeeManage::~FeeManage()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}

bool FeeManage::initDatabaseConnection()
{
    if (m_dbManager->isOpen()) {
        return true;
    }

    QString connName = QString("FeeManage_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    return m_dbManager->initDatabase(connName);
}

void FeeManage::initTableWidget()
{
    tableWidget = new QTableWidget(this);
    tableWidget->setGeometry(30, 290, 1300, 600);
    tableWidget->setColumnCount(3);
    QStringList headers;
    headers << "费用类型" << "费用" << "单位" ;
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void FeeManage::initButtons()
{
    saveBtn = new QPushButton("保存", this);
    saveBtn->setObjectName("pushButton_save");
    saveBtn->setGeometry(1091, 138, 140, 55);
    QFont saveFont = saveBtn->font();
    saveFont.setPointSize(11);
    saveBtn->setFont(saveFont);
    saveBtn->setVisible(false);

    exitBtn = new QPushButton("退出修改", this);
    exitBtn->setObjectName("pushButton_exit");
    exitBtn->setGeometry(1091, 201, 140, 55);
    QFont exitFont = exitBtn->font();
    exitFont.setPointSize(11);
    exitBtn->setFont(exitFont);
    exitBtn->setVisible(false);
}

void FeeManage::initConnections()
{
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FeeManage::onItemSelected);

    connect(ui->pushButton_update, &QPushButton::clicked, this, &FeeManage::onUpdateButtonClicked);
    connect(saveBtn, &QPushButton::clicked, this, &FeeManage::onSaveButtonClicked);
    connect(exitBtn, &QPushButton::clicked, this, &FeeManage::onExitButtonClicked);
}

void FeeManage::refreshData()
{
    clearComboBoxes();

    QString sql = QString("SELECT * FROM item_fee ");
    QVector<QVariantMap> resultList = m_dbManager->executeSelectQuery(sql, {});

    rowToItemIdMap.clear();

    tableWidget->setRowCount(resultList.size());

    int row = 0;
    for (const auto& rowData : resultList) {
        int ItemId = rowData["item_id"].toInt();
        rowToItemIdMap[row] = ItemId;

        QString mm;
        if(ItemId==1) mm="车位费";
        else if(ItemId==2) mm="水费";
        else if(ItemId==3) mm="物业费";
        QTableWidgetItem* item1 = new QTableWidgetItem(mm);
        item1->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, item1);

        QTableWidgetItem* item2 = new QTableWidgetItem(rowData["fee"].toString());
        item2->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, item2);

        QTableWidgetItem* item3 = new QTableWidgetItem(rowData["unit"].toString());
        item3->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 2, item3);

        row++;
    }

    if (isEditing) {
        addEditComboBoxes();
    }
}

void FeeManage::addEditComboBoxes()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QTableWidgetItem* item = tableWidget->item(row, 1);
            QString text = item ? item->text() : "";

            QLineEdit* lineEdit = new QLineEdit(text, this);

            tableWidget->setCellWidget(row, 1, lineEdit);
        }
}

void FeeManage::clearComboBoxes()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QWidget* widget = tableWidget->cellWidget(row, 1);
            if (widget) {
                delete widget;
                tableWidget->setCellWidget(row, 1, nullptr);
            }

            QTableWidgetItem* item = tableWidget->item(row, 1);
            if (item) {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
}

void FeeManage::onItemSelected()
{
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        selectedItemId = -1;
        return;
    }

    int row = selectedRows.first().row();
    selectedItemId = rowToItemIdMap.contains(row) ? rowToItemIdMap[row] : -1;
}

void FeeManage::onUpdateButtonClicked()
{
    isEditing = true;

    ui->pushButton_update->setVisible(false);
    saveBtn->setVisible(true);
    exitBtn->setVisible(true);

    addEditComboBoxes();
}

void FeeManage::onSaveButtonClicked()
{
    for (int row = 0; row < tableWidget->rowCount(); row++) {
            int itemId = rowToItemIdMap.value(row);

            // 从QLineEdit获取值
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(tableWidget->cellWidget(row, 1));
            QString fee = lineEdit ? lineEdit->text().trimmed() : "";

            QString sql = QString("UPDATE item_fee SET "
                                 "fee = %1 "
                                 "WHERE item_id = %2")
                    .arg(fee)
                    .arg(itemId);

            m_dbManager->executeQuery(sql, {});
        }

        QMessageBox::information(this, "成功", "所有修改已保存！");
        refreshData();
}

void FeeManage::onExitButtonClicked()
{
    isEditing = false;

    clearComboBoxes();

    ui->pushButton_update->setVisible(true);
    saveBtn->setVisible(false);
    exitBtn->setVisible(false);

    refreshData();
}
