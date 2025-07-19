#ifndef PROBLEMREPORT_H
#define PROBLEMREPORT_H

#include <QWidget>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProblemReport; }
QT_END_NAMESPACE

class ProblemReport : public QWidget
{
    Q_OBJECT

public:
    ProblemReport(int userId, QWidget *parent = nullptr);
    ~ProblemReport();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ProblemReport *ui;
    int owner_id;
    DatabaseManager *m_dbManager;


    bool initDatabaseConnection();
};
#endif // PROBLEMREPORT_H
