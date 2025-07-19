#ifndef ESTATEREGISTER_H
#define ESTATEREGISTER_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include "databasemanager.h"

namespace Ui {
class EstateRegister;
}

class EstateRegister : public QWidget
{
    Q_OBJECT

public:
    explicit EstateRegister(QWidget *parent = nullptr);
    ~EstateRegister();

private slots:
    void on_btn_save_clicked();

private:
    Ui::EstateRegister *ui;
    DatabaseManager *dbManager;
    bool executeQuery(const QString &sql, const QVariantList &bindValues = QVariantList());
    QString lastError();
    bool validateInput();
};

#endif // ESTATEREGISTER_H
