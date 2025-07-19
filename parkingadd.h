#ifndef PARKINGADD_H
#define PARKINGADD_H

#include <QWidget>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ParkingAdd; }
QT_END_NAMESPACE

class ParkingAdd : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingAdd(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~ParkingAdd();

signals:
    void addButtonClicked();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_exit_clicked();

private:
    Ui::ParkingAdd *ui;
    DatabaseManager* dbManager;
};
#endif // PARKINGADD_H
