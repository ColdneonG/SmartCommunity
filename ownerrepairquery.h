#ifndef OWNERREPAIRQUERY_H
#define OWNERREPAIRQUERY_H

#include "databasemanager.h"
#include <QWidget>
#include <QTableWidget>
#include <QDateTime>

class DatabaseManager;
class QVBoxLayout;

namespace Ui {
class OwnerRepairQuery;
}

class OwnerRepairQuery : public QWidget
{
    Q_OBJECT

public:
    explicit OwnerRepairQuery(int ownerId, QWidget *parent = nullptr);
    ~OwnerRepairQuery();

private slots:
    void on_refreshButton_clicked();

private:
    Ui::OwnerRepairQuery *ui;
    DatabaseManager *m_dbManager;
    QTableWidget *tableWidget;
    int m_currentOwnerId;

    void initTableWidget();
    void loadDataFromDatabase();
    QString getStatusText(bool isProcessed) const;
};

#endif // OWNERREPAIRQUERY_H
