#ifndef PARKINGACCESSQUERY_H
#define PARKINGACCESSQUERY_H

#include <QWidget>
#include "databasemanager.h"
#include <QTableWidget>

namespace Ui {
class ParkingAccessQuery;
}

class ParkingAccessQuery : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingAccessQuery(QWidget *parent = nullptr);
    ~ParkingAccessQuery();


private:
    Ui::ParkingAccessQuery *ui;
    DatabaseManager *dbManager;
    QTableWidget* tableWidget;
    void initTableWidget();
    void initDateEdit();
    void refreshData();
private slots:
    void onStartDateChanged(const QDate &date);
    void on_pushButton_query_clicked();
};

#endif // PARKINGACCESSQUERY_H
