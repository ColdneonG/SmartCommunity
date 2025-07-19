#ifndef OWNERWINDOW_H
#define OWNERWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class OwnerWindow; }
QT_END_NAMESPACE

class OwnerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit OwnerWindow(int userId, QWidget *parent = nullptr);
    ~OwnerWindow();

signals:
    void logoutRequested();

private slots:

    // 菜单折叠逻辑
    void onParkingSpaceManagementClicked();
    void onPaymentManagementClicked();
    void onRepairManagementClicked();

    // 菜单点击统一处理
    void onMenuListClicked(QListWidgetItem *item);

    void on_homeButton_clicked();

    void on_logoutButton_clicked();

private:
    Ui::OwnerWindow *ui;
    int m_userId;
    // 使用 QStackedWidget 管理所有页面
    QStackedWidget *stackedWidget = nullptr;

    // 存储已创建的页面实例
    QMap<QString, QWidget*> pageMap;

    // 初始化页面容器
    void initPageContainer();

    // 创建并添加页面到容器
    void createAndAddPage(const QString &pageName, QWidget *page);

    // 切换到指定页面
    void switchToPage(const QString &pageName);

};
#endif // OWNERWINDOW_H
