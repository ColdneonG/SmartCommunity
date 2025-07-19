#ifndef PARKINGINFORMATION_H
#define PARKINGINFORMATION_H

#include <QWidget>
#include "parkingadd.h"
#include "parkingshow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ParkingInformation; }
QT_END_NAMESPACE

class ParkingInformation : public QWidget
{
    Q_OBJECT

public:
    ParkingInformation(QWidget *parent = nullptr);
    ~ParkingInformation();

private slots:
    void showAddWidget();
    void showShowWidget();

private:
    Ui::ParkingInformation *ui;
    ParkingAdd* widgetAdd;
    ParkingShow* widgetShow;
};
#endif // PARKINGINFORMATION_H
