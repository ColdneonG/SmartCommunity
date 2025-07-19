#ifndef OWNERMODIFY_H
#define OWNERMODIFY_H

#include <QWidget>
#include <QVector>
#include <QVariantMap>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OwnerModify; }
QT_END_NAMESPACE

class OwnerModify : public QWidget
{
    Q_OBJECT

public:
    OwnerModify(QWidget *parent = nullptr);
    ~OwnerModify();

private slots:
    void on_modifyButton_clicked();
    void on_deleteButton_clicked();
    void on_refreshButton_clicked();
    void on_tableWidget_itemSelectionChanged();

private:
    Ui::OwnerModify *ui;
    DatabaseManager *dbManager; // 使用数据库管理类

    void initUI(); // 初始化界面
    bool loadAllOwners(); // 加载所有业主信息
    void showOwners(const QVector<QVariantMap>& owners); // 显示业主到表格
    void updateButtonState(bool hasSelection); // 更新按钮状态
};
#endif // OWNERMODIFY_H
