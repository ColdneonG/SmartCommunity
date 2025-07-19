#ifndef PARKINGQUERY_H
#define PARKINGQUERY_H

#include <QWidget>
#include <QLabel>
#include "databasemanager.h"

namespace Ui {
class ParkingQuery;
}

class ParkingQuery : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingQuery(int ownerId,QWidget *parent = nullptr);
    ~ParkingQuery();

private:
    Ui::ParkingQuery *ui;
    void init();
    int ownerId;
    DatabaseManager *dbManager;
    QLabel* applyLabel;
};

#endif // PARKINGQUERY_H
