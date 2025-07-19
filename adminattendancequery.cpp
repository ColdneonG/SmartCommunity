#include "adminattendancequery.h"
#include "ui_adminattendancequery.h"
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDateTime>

AdminAttendanceQuery::AdminAttendanceQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminAttendanceQuery),
    m_dbManager(new DatabaseManager(this)),
    tableWidget(nullptr)
{
    QString connName = QString("AdminAttendanceQuery_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    if (!m_dbManager->initDatabase(connName)) {
        qDebug() << "数据库初始化失败";
        QMessageBox::critical(this, "错误", "数据库连接失败，请检查配置");
    }

    initTableWidget();
    loadDataFromDatabase();
}

AdminAttendanceQuery::~AdminAttendanceQuery()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
    }
    delete ui;
}

void AdminAttendanceQuery::initTableWidget()
{
    // 设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 初始化表格
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(8);
    QStringList headers;
    headers << "出勤ID" << "员工ID" << "用户名" << "出勤日期"
            << "签到时间" << "签退时间" << "状态" << "角色";
    tableWidget->setHorizontalHeaderLabels(headers);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 按钮布局
    buttonLayout->addStretch();
    buttonLayout->addWidget(ui->refreshButton);
    buttonLayout->addWidget(ui->detailButton);
    buttonLayout->addStretch();
    buttonLayout->setContentsMargins(30, 60, 30, 20);

    // 主布局组装
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(tableWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    setLayout(mainLayout);
}

void AdminAttendanceQuery::loadDataFromDatabase()
{
    tableWidget->clearContents();
    tableWidget->setRowCount(0);

    if (!m_dbManager->isOpen()) {
        qDebug() << "数据库未打开";
        // 尝试重新连接数据库
        QString connName = QString("AdminAttendanceQuery_%1")
                   .arg(QDateTime::currentMSecsSinceEpoch());

        if (!m_dbManager->initDatabase(connName)) {
            QMessageBox::warning(this, "提示", "数据库连接失败，无法加载数据");
            return;
        }
    }

    // 关联查询出勤表和用户表，筛选管理员(0)和物业人员(1)
    QString queryStr = "SELECT a.attendance_id, a.staff_id, u.username, a.attendance_date, "
                       "a.check_in_time, a.check_out_time, a.status, u.role "
                       "FROM attendance a "
                       "LEFT JOIN users u ON a.staff_id = u.id "
                       "WHERE u.role IN (0, 1)";

    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr);

    if (result.isEmpty()) {
        QString error = m_dbManager->lastError();
        if (!error.isEmpty()) {
            qDebug() << "查询失败:" << error;
            QMessageBox::warning(this, "查询失败", QString("无法获取出勤数据: %1").arg(error));
        } else {
            QMessageBox::information(this, "提示", "没有找到任何出勤记录");
        }
        return;
    }

    // 填充表格数据
    for (const auto& row : result) {
        if (!row.contains("attendance_id") || !row.contains("staff_id")) {
            qDebug() << "跳过无效记录，缺少必要字段";
            continue;
        }

        int rowIndex = tableWidget->rowCount();
        tableWidget->insertRow(rowIndex);

        tableWidget->setItem(rowIndex, 0, new QTableWidgetItem(row["attendance_id"].toString()));
        tableWidget->setItem(rowIndex, 1, new QTableWidgetItem(row["staff_id"].toString()));
        tableWidget->setItem(rowIndex, 2, new QTableWidgetItem(row["username"].toString()));
        tableWidget->setItem(rowIndex, 3, new QTableWidgetItem(row["attendance_date"].toString()));
        tableWidget->setItem(rowIndex, 4, new QTableWidgetItem(row["check_in_time"].toString()));
        tableWidget->setItem(rowIndex, 5, new QTableWidgetItem(row["check_out_time"].toString()));
        tableWidget->setItem(rowIndex, 6, new QTableWidgetItem(getStatusText(row["status"].toInt())));

        // 角色转换
        QString roleStr = (row["role"].toInt() == 0) ? "管理员" : "物业工作人员";
        tableWidget->setItem(rowIndex, 7, new QTableWidgetItem(roleStr));
    }

    qDebug() << "数据加载完成，共" << tableWidget->rowCount() << "条记录";
}

QString AdminAttendanceQuery::getStatusText(int status) const
{
    switch (status) {
    case 1: return "正常";
    case 2: return "迟到/早退";
    case 0:
    default: return "缺勤";
    }
}

void AdminAttendanceQuery::on_refreshButton_clicked()
{
    loadDataFromDatabase();
}

void AdminAttendanceQuery::on_detailButton_clicked()
{
    // 获取选中行的员工ID
    QModelIndexList selected = tableWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一行记录");
        return;
    }

    int row = selected.first().row();
    QTableWidgetItem *item = tableWidget->item(row, 1);
    if (!item) {
        QMessageBox::warning(this, "错误", "无法获取员工ID");
        return;
    }

    // 显示详情对话框，调整大小为1360x900
    int staffId = item->text().toInt();
    AttendanceDetailDialog *detailDialog = new AttendanceDetailDialog(staffId, this);
    detailDialog->setWindowTitle(QString("员工ID: %1 出勤详情").arg(staffId));
    detailDialog->resize(1360, 900);

    // 设置对话框关闭时自动删除
    detailDialog->setAttribute(Qt::WA_DeleteOnClose);

    // 连接对话框销毁信号，确保主窗口检查数据库状态
    connect(detailDialog, &QObject::destroyed, this, [this]() {
        if (!m_dbManager->isOpen()) {
            qDebug() << "详情对话框关闭后，主窗口尝试重新连接数据库";
            QString connName = QString("detailDialog_%1")
                       .arg(QDateTime::currentMSecsSinceEpoch());
            m_dbManager->initDatabase(connName);
        }
    });

    detailDialog->show();
}

// 详情对话框实现
AttendanceDetailDialog::AttendanceDetailDialog(int staffId, QWidget *parent)
    : QDialog(parent), m_dbManager(new DatabaseManager(this)), detailTable(nullptr)
{
    // 设置对话框属性
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);

    QString connName = QString("detailDialog_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    if (!m_dbManager->initDatabase(connName)) {
        qDebug() << "详情对话框数据库初始化失败";
        QMessageBox::critical(this, "错误", "数据库连接失败");
        return;
    }

    initTable();
    loadDetailData(staffId);
}

AttendanceDetailDialog::~AttendanceDetailDialog()
{
    if (m_dbManager) {
        m_dbManager->closeDatabase();
        delete m_dbManager;
        m_dbManager = nullptr;
    }
}

void AttendanceDetailDialog::initTable()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    detailTable = new QTableWidget(this);
    detailTable->setColumnCount(6);
    QStringList headers;
    headers << "出勤ID" << "员工ID" << "出勤日期" << "签到时间" << "签退时间" << "状态";
    detailTable->setHorizontalHeaderLabels(headers);
    detailTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    detailTable->verticalHeader()->setVisible(false);
    detailTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加关闭按钮
    QPushButton *closeButton = new QPushButton("关闭", this);
    closeButton->setStyleSheet("QPushButton {"
                               "    padding: 10px 20px;"
                               "    text-align: center;"
                               "    text-decoration: none;"
                               "    display: inline-block;"
                               "    font-size: 14px;"
                               "    margin: 4px 2px;"
                               "    cursor: pointer;"
                               "    border-radius: 4px;"
                               "}");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    // 添加表格和按钮到主布局
    layout->addWidget(detailTable);
    layout->addLayout(buttonLayout);
    layout->setContentsMargins(10, 10, 10, 10);

    setLayout(layout);
}

void AttendanceDetailDialog::loadDetailData(int staffId)
{
    detailTable->clearContents();
    detailTable->setRowCount(0);

    if (!m_dbManager->isOpen()) {
        QMessageBox::warning(this, "提示", "数据库未连接");
        return;
    }

    // 查询指定员工的所有出勤记录
    QString queryStr = "SELECT attendance_id, staff_id, attendance_date, "
                       "check_in_time, check_out_time, status "
                       "FROM attendance "
                       "WHERE staff_id = :staffId";

    QVector<QVariantMap> result = m_dbManager->executeSelectQuery(queryStr, {staffId});

    if (result.isEmpty()) {
        QString error = m_dbManager->lastError();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "查询失败", error);
        } else {
            QMessageBox::information(this, "提示", "没有找到该员工的出勤记录");
        }
        return;
    }

    // 填充详情表格
    for (const auto& row : result) {
        int rowIndex = detailTable->rowCount();
        detailTable->insertRow(rowIndex);

        detailTable->setItem(rowIndex, 0, new QTableWidgetItem(row["attendance_id"].toString()));
        detailTable->setItem(rowIndex, 1, new QTableWidgetItem(row["staff_id"].toString()));
        detailTable->setItem(rowIndex, 2, new QTableWidgetItem(row["attendance_date"].toString()));
        detailTable->setItem(rowIndex, 3, new QTableWidgetItem(row["check_in_time"].toString()));
        detailTable->setItem(rowIndex, 4, new QTableWidgetItem(row["check_out_time"].toString()));
        detailTable->setItem(rowIndex, 5, new QTableWidgetItem(getStatusText(row["status"].toInt())));
    }
}

QString AttendanceDetailDialog::getStatusText(int status) const
{
    switch (status) {
    case 1: return "正常";
    case 2: return "迟到/早退";
    case 0:
    default: return "缺勤";
    }
}
