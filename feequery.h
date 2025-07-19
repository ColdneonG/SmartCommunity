#ifndef FEEQUERY_H
#define FEEQUERY_H

#include <QWidget>
#include <QTableWidget>
#include <QDate>
#include "databasemanager.h"

class Database;

namespace Ui {
class FeeQuery;
}

class FeeQuery : public QWidget
{
    Q_OBJECT

public:
    explicit FeeQuery(int owner_id, QWidget *parent = nullptr);
    ~FeeQuery();

private slots:
    void onStartDateChanged(const QDate &date);
    void on_pushButton_query_clicked();

    void on_pushButton_query_2_clicked();

private:
    Ui::FeeQuery *ui;
    int owner_id;
    QTableWidget *tableWidget;
    DatabaseManager *m_dbManager;
    int selectedFeeId=-1;
    QMap<int,int> rowToFeeIdMap;

    void initTableWidget();
    void refreshData();
    void initDateEdit();
    void onItemSelected();

    bool initDatabaseConnection();
};

#endif // FEEQUERY_H
