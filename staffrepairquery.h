#ifndef STAFFREPAIRQUERY_H
#define STAFFREPAIRQUERY_H

#include <QWidget>
#include "databasemanager.h"
#include <QTableWidget>

namespace Ui {
class StaffRepairQuery;
}

class StaffRepairQuery : public QWidget
{
    Q_OBJECT

public:
    explicit StaffRepairQuery(int userId, QWidget *parent = nullptr);
    ~StaffRepairQuery();

private slots:
    void on_refreshButton_clicked();

private:
    Ui::StaffRepairQuery *ui;
    DatabaseManager* m_dbManager;
    QTableWidget* tableWidget;
    int m_currentUserId; // 当前处理用户ID

    void initTableWidget();
    void loadDataFromDatabase();
    QString getStatusText(bool isProcessed) const;
};

#endif // STAFFREPAIRQUERY_H
