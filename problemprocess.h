#ifndef PROBLEMPROCESS_H
#define PROBLEMPROCESS_H

#include <QWidget>
#include <QTableWidget>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProblemProcess; }
QT_END_NAMESPACE

class ProblemProcess : public QWidget
{
    Q_OBJECT

public:
    explicit ProblemProcess(int userId, QWidget *parent = nullptr);
    ~ProblemProcess();

private slots:
    void on_refreshButton_clicked();
    void on_processButton_clicked();

private:
    Ui::ProblemProcess *ui;
    DatabaseManager* m_dbManager;
    QTableWidget* tableWidget;
    int m_currentUserId; // 当前处理用户ID

    void initTableWidget();
    void loadDataFromDatabase();
    QString getStatusText(bool isProcessed) const;
    void updateProcessStatus(const QString& orderId, const QString& processDesc);
};
#endif // PROBLEMPROCESS_H
