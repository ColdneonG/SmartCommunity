#include "login.h"
#include "adminwindow.h"
#include "staffwindow.h"
#include "ownerwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Login login;  // 登录窗口
    QWidget* currentWindow = nullptr;  // 记录当前显示的窗口

    // 处理登录成功：创建对应窗口
    QObject::connect(&login, &Login::loginSuccess, [&](int userId, int role) {
        // 销毁之前的窗口，避免内存泄漏
        if (currentWindow) {
            currentWindow->deleteLater();
            currentWindow = nullptr;
        }



        // 根据角色创建窗口，并绑定登出信号
        if (role == 0) {  // 管理员窗口
            AdminWindow* admin = new AdminWindow(userId);
            currentWindow = admin;
            // 登出逻辑：销毁当前窗口，显示登录界面（通过接口清空输入框）
            QObject::connect(admin, &AdminWindow::logoutRequested, [&]() {
                currentWindow->deleteLater();
                currentWindow = nullptr;
                login.clearInput();  // 调用公有接口清空输入框
                login.show();
            });
        }
        else if (role == 1) {  // 工作人员窗口
            StaffWindow* staff = new StaffWindow(userId);
            currentWindow = staff;
            QObject::connect(staff, &StaffWindow::logoutRequested, [&]() {
                currentWindow->deleteLater();
                currentWindow = nullptr;
                login.clearInput();  // 调用接口
                login.show();
            });
        }
        else if (role == 2) {  // 业主窗口
            OwnerWindow* owner = new OwnerWindow(userId);
            currentWindow = owner;
            QObject::connect(owner, &OwnerWindow::logoutRequested, [&]() {
                currentWindow->deleteLater();
                currentWindow = nullptr;
                login.clearInput();  // 调用接口
                login.show();
            });
        }

        // 显示新窗口，隐藏登录界面
        if (currentWindow) {
            currentWindow->show();
            login.hide();
        }
    });

    // 登录窗口关闭时退出程序
    QObject::connect(&login, &Login::rejected, &a, &QApplication::quit);

    login.show();
    return a.exec();
}
