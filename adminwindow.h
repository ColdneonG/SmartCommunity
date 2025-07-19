#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H
#include <QMainWindow>
#include <QSqlQueryModel>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QMap>
//#include "database.h"

namespace Ui {
class AdminWindow;
}

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(int userId, QWidget *parent = nullptr); // 接收管理员ID
    ~AdminWindow();

signals:
    void logoutRequested();

private slots:
    void onStaffManagementClicked();
    void onSystemSettingClicked();

    // 系统设置相关
    void onSystemInitClicked();    // 系统初始化

    // 菜单点击槽函数
    void onlistWidget_menuClicked(QListWidgetItem *item);

    void on_homeButton_clicked();

    void on_logoutButton_clicked();

private:
    Ui::AdminWindow *ui;
    int m_adminId;         // 当前管理员ID
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
#endif // ADMINWINDOW_H
