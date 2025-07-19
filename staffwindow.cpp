#include "staffwindow.h"
#include "ui_staffwindow.h"
#include "personnel/attendancerecord.h"
#include "personnel/leaveapply.h"
#include "personnel/myattendancequery.h"
#include "repair/staffrepairquery.h"
#include "repair/problemprocess.h"
#include "owner_manage/ownerregister.h"
#include "owner_manage/familymember.h"
#include "owner_manage/ownerquery.h"
#include "owner_manage/estateregister.h"
#include "owner_manage/ownermodify.h"
#include "parking/parkingaccessquery.h"
#include "parking/parkinginformation.h"
#include "parking/parkingrental.h"
#include "parking/parkingrentalmanage.h"
#include "parking/parkingaccess.h"
#include "payment/feemanage.h"
#include "payment/freequerystaff.h"
#include "staffhome.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>

StaffWindow::StaffWindow(int userId, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::StaffWindow),
    m_userId(userId)
{
    ui->setupUi(this);
    setWindowTitle("物业工作人员界面");

    initPageContainer();

    QListWidgetItem* parentItem1 = ui->listWidget_menu->item(0);
    parentItem1->setData(Qt::UserRole, 0);
    for (int i = 1; i <= 3; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
    }
    QListWidgetItem* parentItem2 = ui->listWidget_menu->item(4);
    parentItem2->setData(Qt::UserRole, 0);
    for (int i = 5; i <= 8; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
    }
    QListWidgetItem* parentItem3 = ui->listWidget_menu->item(9);
    parentItem3->setData(Qt::UserRole, 0);
    for (int i = 10; i <= 13; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
    }
    QListWidgetItem* parentItem4 = ui->listWidget_menu->item(14);
    parentItem4->setData(Qt::UserRole, 0);
    for (int i = 15; i <= 16; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
    }
    QListWidgetItem* parentItem5 = ui->listWidget_menu->item(17);
    parentItem5->setData(Qt::UserRole, 0);
    for (int i = 18; i <= 19; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
    }

    switchToPage("回到主页");

    connect(ui->listWidget_menu, &QListWidget::itemClicked, this, &StaffWindow::onlistWidget_menuClicked);
}

void StaffWindow::initPageContainer()
{
    stackedWidget = new QStackedWidget(ui->rightContainer); // 右侧显示区域
    QVBoxLayout *layout = new QVBoxLayout(ui->rightContainer);
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距
    layout->addWidget(stackedWidget);
}

// 创建并添加页面
void StaffWindow::createAndAddPage(const QString &pageName, QWidget *page)
{
    qDebug() << "创建页面成功";
    page->setWindowFlags(Qt::FramelessWindowHint); // 无边框
    page->setAttribute(Qt::WA_TranslucentBackground, false);
    stackedWidget->addWidget(page);
    pageMap[pageName] = page; // 缓存页面
}

// 切换页面
void StaffWindow::switchToPage(const QString &pageName)
{
    if (!currentPageName.isEmpty() && pageMap.contains(currentPageName)) {
       QWidget *oldPage = pageMap[currentPageName];
       // 从栈窗口中移除旧页面
       stackedWidget->removeWidget(oldPage);
       // 从映射中删除
       pageMap.remove(currentPageName);
       // 销毁旧页面
       oldPage->deleteLater(); // 安全删除（延迟到事件循环处理）
       qDebug() << "销毁页面：" << currentPageName;
       }

    // 页面不存在则创建（按需加载）
    if (!pageMap.contains(pageName)) {
        // 出勤相关页面
        if (pageName == "月度出勤") {
            createAndAddPage(pageName, new MyAttendanceQuery(m_userId));
        } else if (pageName == "请销假") {
            createAndAddPage(pageName, new LeaveApply(m_userId));
        } else if (pageName == "出勤登记") {
            createAndAddPage(pageName, new AttendanceRecord(m_userId));
        }
        // 业主管理相关页面
        else if (pageName == "业主登记") {
            createAndAddPage(pageName, new OwnerRegister());
        } else if (pageName == "房产登记") {
            createAndAddPage(pageName, new EstateRegister());
        } else if (pageName == "业主查询") {
            createAndAddPage(pageName, new OwnerQuery());
        } else if (pageName == "业主信息修改") {
            createAndAddPage(pageName, new OwnerModify());
        }
        // 车位管理相关页面
        else if (pageName == "车位信息") {
            createAndAddPage(pageName, new ParkingInformation());
        } else if (pageName == "车位出租") {
            createAndAddPage(pageName, new ParkingRental());
        } else if (pageName == "出租管理") {
            createAndAddPage(pageName, new ParkingRentalManage());
        } else if (pageName == "进出查询") {
            createAndAddPage(pageName, new ParkingAccessQuery());
        }
        // 缴费管理相关页面
       else if (pageName == "价格维护") {
            createAndAddPage(pageName, new FeeManage());
        } else if (pageName == "缴费查询登记") {
            createAndAddPage(pageName, new FreeQueryStaff());
        }
        // 维修管理相关页面
        else if (pageName == "维修查询") {
            createAndAddPage(pageName, new StaffRepairQuery(m_userId));
        } else if (pageName == "维修进度登记") {
            createAndAddPage(pageName, new ProblemProcess(m_userId));
        }
        else if (pageName == "模拟车辆进出") {
            createAndAddPage(pageName, new ParkingAccess());
        }
        else if (pageName == "回到主页") {
            createAndAddPage(pageName, new StaffHome(m_userId));
        }
    }
    // 切换到目标页面
    if (pageMap.contains(pageName)) {
        qDebug() << "切换页面成功";
        stackedWidget->setCurrentWidget(pageMap[pageName]);
    } else {
        qDebug() << "Error: Page" << pageName << "not found!";
    }
}

void StaffWindow::onAttendanceLeaveClicked() {
    bool isHidden = ui->listWidget_menu->item(1)->isHidden();
    for (int i = 1; i <= 3; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

void StaffWindow::onOwnerManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(5)->isHidden();
    for (int i = 5; i <= 8; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

void StaffWindow::onParkingManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(10)->isHidden();
    for (int i = 10; i <= 13; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

void StaffWindow::onPaymentManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(15)->isHidden();
    for (int i = 15; i <= 16; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

void StaffWindow::onRepairManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(18)->isHidden();
    for (int i = 18; i <= 19; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

void StaffWindow::onlistWidget_menuClicked(QListWidgetItem *item) {
    QString text = item->text();

    if (text == "出勤请假") {
        onAttendanceLeaveClicked();
    } else if (text == "    月度出勤") {
        switchToPage("月度出勤");
    } else if (text == "    请销假") {
        switchToPage("请销假");
    } else if (text == "    出勤登记") {
        switchToPage("出勤登记");
    }
    else if (text == "业主管理") {
        onOwnerManagementClicked();
    } else if (text == "    业主登记") {
        switchToPage("业主登记");
    } else if (text == "    房产登记") {
        switchToPage("房产登记");
    } else if (text == "    业主查询") {
        switchToPage("业主查询");
    } else if (text == "    业主信息修改") {
        switchToPage("业主信息修改");
    }
    else if (text == "车位管理") {
        onParkingManagementClicked();
    } else if (text == "    车位信息") {
        switchToPage("车位信息");
    } else if (text == "    车位出租") {
        switchToPage("车位出租");
    } else if (text == "    出租管理") {
        switchToPage("出租管理");
    } else if (text == "    进出查询") {
        switchToPage("进出查询");
    }
    else if (text == "缴费管理") {
        onPaymentManagementClicked();
    } else if (text == "    价格维护") {
        switchToPage("价格维护");
    } else if (text == "    缴费查询登记") {
        switchToPage("缴费查询登记");
    }
    else if (text == "维修管理") {
        onRepairManagementClicked();
    } else if (text == "    维修查询") {
        switchToPage("维修查询");
    } else if (text == "    维修进度登记") {
        switchToPage("维修进度登记");
    } else if (text == "模拟车辆进出") {
        switchToPage("模拟车辆进出");
    }
}


StaffWindow::~StaffWindow() { delete ui; }

void StaffWindow::on_homeButton_clicked()
{
    switchToPage("回到主页");
}

void StaffWindow::on_logoutButton_clicked()
{
    if (QMessageBox::question(this, "确认登出", "确定要退出当前账号吗？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit logoutRequested();  // 发送登出信号
        this->close();           // 关闭当前窗口
    }
}
