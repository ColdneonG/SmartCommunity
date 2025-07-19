#ifndef PARKINGACCESS_H
#define PARKINGACCESS_H

#include <QWidget>

#include "databasemanager.h"

namespace Ui {
class ParkingAccess;
}

class ParkingAccess : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingAccess(QWidget *parent = nullptr);
    ~ParkingAccess();

private slots:
    void on_pushButton_ok_clicked();

private:
    Ui::ParkingAccess *ui;
    DatabaseManager *dbManager;
};

#endif // PARKINGACCESS_H
