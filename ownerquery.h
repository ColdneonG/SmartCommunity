#ifndef OWNERQUERY_H
#define OWNERQUERY_H

#include <QWidget>
#include <QVector>
#include <QVariantMap>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OwnerQuery; }
QT_END_NAMESPACE

class OwnerQuery : public QWidget
{
    Q_OBJECT

public:
    OwnerQuery(QWidget *parent = nullptr);
    ~OwnerQuery();

private slots:
    void on_searchButton_clicked(); // 查询按钮点击槽函数

private:
    Ui::OwnerQuery *ui;
    DatabaseManager *dbManager;  // 数据库管理实例
    void initUI();  // 初始化UI组件
    void showQueryResult(const QVector<QVariantMap>& resultList);  // 显示查询结果
};

#endif // OWNERQUERY_H
