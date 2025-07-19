#ifndef PARKINGAPPLY_H
#define PARKINGAPPLY_H

#include <QWidget>
#include "databasemanager.h"

namespace Ui {
class ParkingApply;
}

class ParkingApply : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingApply(int ownerid,QWidget *parent = nullptr);
    ~ParkingApply();

private slots:
    void on_pushButton_apply_clicked();

private:
    Ui::ParkingApply *ui;
    int m_ownerid;
    DatabaseManager *dbManager;
};

#endif // PARKINGAPPLY_H
