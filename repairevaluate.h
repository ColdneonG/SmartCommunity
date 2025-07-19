#ifndef REPAIREVALUATE_H
#define REPAIREVALUATE_H

#include <QWidget>
#include <QTableWidget>
#include <QMap>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RepairEvaluate; }
QT_END_NAMESPACE

class RepairEvaluate : public QWidget
{
    Q_OBJECT

public:
    RepairEvaluate(int ownerId, QWidget *parent = nullptr);
    ~RepairEvaluate();

private slots:
    void on_refreshButton_clicked();
    void on_evaluateButton_clicked();

private:
    Ui::RepairEvaluate *ui;
    DatabaseManager *m_dbManager;
    QTableWidget *tableWidget;
    int m_currentOwnerId;
    QMap<QString, QString> m_orderStaffMap; // 存储订单ID与处理人ID的映射

    void initTableWidget();
    void loadDataFromDatabase();
    QString getStatusText(bool isProcessed) const;
    bool hasEvaluated(const QString &orderId);
    void saveEvaluation(const QString &orderId, int score, const QString &content, const QString &staffId);
};
#endif // REPAIREVALUATE_H
