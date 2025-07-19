#ifndef ADDSTAFF_H
#define ADDSTAFF_H

#include <QWidget>
#include "databasemanager.h"

namespace Ui {
class AddStaff;
}

class AddStaff : public QWidget
{
    Q_OBJECT

public:
    explicit AddStaff(QWidget *parent = nullptr);
    ~AddStaff();

private slots:
    void on_addButton_clicked();

signals:
    void addSuccess(qint64 staffId);

private:
    Ui::AddStaff *ui;
    DatabaseManager *dbManager;
    bool validateInput();
    bool checkPasswordMatch();
    void clearFormFields();
};

#endif // ADDSTAFF_H
