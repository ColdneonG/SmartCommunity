#ifndef PARKINGRENTAL_H
#define PARKINGRENTAL_H

#include <QWidget>
#include <QTableWidget>
#include <QMap>
#include "databasemanager.h"

namespace Ui {
class ParkingRental;
}

class ParkingRental : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingRental(QWidget *parent = nullptr);
    ~ParkingRental();

private slots:
    void on_pushButton_rental_clicked();
    void onItemSelected();

private:
    Ui::ParkingRental *ui;
    DatabaseManager *dbManager;
    QTableWidget* tableWidget;
    QMap<int,int> rowToApplyIdMap;
    int applyId=-1;
    void initTableWidget();
    void refreshApplyData();

};

#endif // PARKINGRENTAL_H
