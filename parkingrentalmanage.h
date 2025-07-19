#ifndef PARKINGRENTALMANAGE_H
#define PARKINGRENTALMANAGE_H

#include <QWidget>
#include "databasemanager.h"
#include <QTableWidget>
#include <QMap>
namespace Ui {
class ParkingRentalManage;
}

class ParkingRentalManage : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingRentalManage(QWidget *parent = nullptr);
    ~ParkingRentalManage();

private:
    Ui::ParkingRentalManage *ui;
    DatabaseManager *dbManager;
    QTableWidget* tableWidget;
    int selectedRentalId=-1;
    QMap<int,int> rowToRentalIdMap;
    void initTableWidget();
    void initConnections();
    void refreshData();
private slots:
    void onItemSelected();

    void on_pushButton_delete_clicked();
    void on_pushButton_continue_clicked();
};

#endif // PARKINGRENTALMANAGE_H
