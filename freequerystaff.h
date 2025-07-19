#ifndef FREEQUERYSTAFF_H
#define FREEQUERYSTAFF_H

#include <QWidget>
#include <QMap>
#include <QTableWidget>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FreeQueryStaff; }
QT_END_NAMESPACE

class FreeQueryStaff : public QWidget
{
    Q_OBJECT

public:
    explicit FreeQueryStaff(QWidget *parent = nullptr);
    ~FreeQueryStaff();

private slots:
    void on_pushButton_query_clicked();    // 查询按钮点击
    void on_pushButton_query_2_clicked();  // 缴费操作按钮点击
    void onStartDateChanged(const QDate &date);  // 开始日期变化

private:
    Ui::FreeQueryStaff *ui;
    DatabaseManager *m_dbManager;  // 数据库管理器
    QTableWidget *tableWidget;     // 表格控件
    int selectedFeeId;             // 当前选中的费用ID
    QMap<int, int> rowToFeeIdMap;  // 行号到fee_id的映射

    void initUI();                 // 初始化UI组件
    void showFeeLogs(const QVector<QVariantMap> &feeLogs);  // 显示费用记录
    void refreshData();
    void onItemSelected(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif // FREEQUERYSTAFF_H
