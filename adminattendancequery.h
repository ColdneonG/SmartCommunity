#ifndef ADMINATTENDANCEQUERY_H
#define ADMINATTENDANCEQUERY_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include "databasemanager.h"

namespace Ui {
class AdminAttendanceQuery;
}

class AdminAttendanceQuery : public QWidget
{
    Q_OBJECT

public:
    explicit AdminAttendanceQuery(QWidget *parent = nullptr);
    ~AdminAttendanceQuery();

private slots:
    void on_refreshButton_clicked();
    void on_detailButton_clicked();

private:
    Ui::AdminAttendanceQuery *ui;
    DatabaseManager *m_dbManager;
    QTableWidget *tableWidget;
    void initTableWidget();
    void loadDataFromDatabase();
    QString getStatusText(int status) const;
};

// 出勤详情对话框
class AttendanceDetailDialog : public QDialog
{
    Q_OBJECT
public:
    AttendanceDetailDialog(int staffId, QWidget *parent = nullptr);
    ~AttendanceDetailDialog() override;

private:
    DatabaseManager *m_dbManager;
    QTableWidget *detailTable;
    void initTable();
    void loadDetailData(int staffId);
    QString getStatusText(int status) const;
};

#endif // ADMINATTENDANCEQUERY_H
