#ifndef STAFFWINDOW_H
#define STAFFWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>
#include "databasemanager.h"
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class StaffWindow; }
QT_END_NAMESPACE

class StaffWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StaffWindow(int userId, QWidget *parent = nullptr);
    ~StaffWindow();

signals:
    void logoutRequested();

private slots:
    // 菜单折叠逻辑（与OwnerWindow一致）
    void onAttendanceLeaveClicked();
    void onOwnerManagementClicked();
    void onRepairManagementClicked();
    void onPaymentManagementClicked();
    void onParkingManagementClicked();
    // 菜单点击统一处理（核心分发逻辑）
    void onlistWidget_menuClicked(QListWidgetItem *item);

    void on_homeButton_clicked();

    void on_logoutButton_clicked();

private:
    Ui::StaffWindow *ui;
    int m_userId; // 当前工作人员ID
    // 页面管理
    QStackedWidget *stackedWidget = nullptr; // 管理所有功能页面
    QMap<QString, QWidget*> pageMap; // 缓存已创建的页面
    QString currentPageName;

    // 页面容器初始化
    void initPageContainer();
    // 创建并添加页面到容器
    void createAndAddPage(const QString &pageName, QWidget *page);
    // 切换到指定页面
    void switchToPage(const QString &pageName);
};

#endif // STAFFWINDOW_H
