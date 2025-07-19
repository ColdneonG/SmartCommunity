#include "ownerwindow.h"
#include "ui_ownerwindow.h"
#include "repair/problemreport.h"
#include "repair/ownerrepairquery.h"
#include "repair/repairevaluate.h"
#include "parking/parkingapply.h"
#include "parking/parkingquery.h"
#include "payment/feequery.h"
#include "ownerhome.h"
#include <QDebug>
#include <QMessageBox>

OwnerWindow::OwnerWindow(int userId, QWidget *parent)
    : QMainWindow(parent)
   , ui(new Ui::OwnerWindow),
      m_userId(userId)
{
    ui->setupUi(this);
    setWindowTitle("业主界面");

    initPageContainer();

    // 初始化菜单层级（0：父项，1：子项）
    QListWidgetItem* parentItem1 = ui->listWidget_menu->item(0);
    parentItem1->setData(Qt::UserRole, 0);
    for (int i = 1; i <= 2; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
        //childItem1->setHidden(true); // 默认隐藏子项
    }

    QListWidgetItem* parentItem2 = ui->listWidget_menu->item(3);
    parentItem2->setData(Qt::UserRole, 0);
    QListWidgetItem* childItem1 = ui->listWidget_menu->item(4);
    childItem1->setData(Qt::UserRole, 1);


    QListWidgetItem* parentItem3 = ui->listWidget_menu->item(5);
    parentItem3->setData(Qt::UserRole, 0);
    for (int i = 6; i <= 8; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1);
        //childItem1->setHidden(true); // 默认隐藏子项
    }

    switchToPage("回到主页");

    // 连接菜单点击信号
    connect(ui->listWidget_menu, &QListWidget::itemClicked, this, &OwnerWindow::onMenuListClicked);
}

OwnerWindow::~OwnerWindow()
{
    delete ui;
}

// 初始化页面容器
void OwnerWindow::initPageContainer()
{
    stackedWidget = new QStackedWidget(ui->rightContainer); // 右侧显示区域
    QVBoxLayout *layout = new QVBoxLayout(ui->rightContainer);
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距
    layout->addWidget(stackedWidget);
}

// 创建并添加页面
void OwnerWindow::createAndAddPage(const QString &pageName, QWidget *page)
{
    qDebug() << "创建页面：" << pageName;
    page->setWindowFlags(Qt::FramelessWindowHint); // 无边框
    page->setAttribute(Qt::WA_TranslucentBackground, false);
    stackedWidget->addWidget(page);
    pageMap[pageName] = page; // 缓存页面
}

// 切换页面
void OwnerWindow::switchToPage(const QString &pageName)
{
    // 页面不存在则创建（按需加载）
    if (!pageMap.contains(pageName)) {
        if (pageName == "车位申请") {
            createAndAddPage(pageName, new ParkingApply(m_userId));
        } else if (pageName == "我的车位") {
            createAndAddPage(pageName, new ParkingQuery(m_userId));
        } else if (pageName == "我的缴费") {
            createAndAddPage(pageName, new FeeQuery(m_userId));
        } else if (pageName == "故障报修") {
            createAndAddPage(pageName, new ProblemReport(m_userId));
        } else if (pageName == "维修进度查询") {
            createAndAddPage(pageName, new OwnerRepairQuery(m_userId));
        } else if (pageName == "维修评价") {
            createAndAddPage(pageName, new RepairEvaluate(m_userId));
        } else if (pageName == "回到主页") {
            createAndAddPage(pageName, new OwnerHome(m_userId));
        }
    }
    // 切换到目标页面
    if (pageMap.contains(pageName)) {
        qDebug() << "切换到页面：" << pageName;
        stackedWidget->setCurrentWidget(pageMap[pageName]);
    } else {
        qDebug() << "错误：页面" << pageName << "不存在！";
    }
}

// 车位管理父项点击（折叠/展开子菜单）
void OwnerWindow::onParkingSpaceManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(1)->isHidden();
    for (int i = 1; i <= 2; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

// 缴费管理父项点击（折叠/展开子菜单）
void OwnerWindow::onPaymentManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(4)->isHidden();
    ui->listWidget_menu->item(4)->setHidden(!isHidden);
}

// 维修管理父项点击（折叠/展开子菜单）
void OwnerWindow::onRepairManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(6)->isHidden();
    for (int i = 6; i <= 8; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden);
    }
}

// 菜单点击统一处理
void OwnerWindow::onMenuListClicked(QListWidgetItem *item)
{
    QString text = item->text();
    // 父项菜单（折叠/展开）
    if (text == "车位管理") {
        onParkingSpaceManagementClicked();
    } else if (text == "缴费管理") {
        onPaymentManagementClicked();
    } else if (text == "维修管理") {
        onRepairManagementClicked();
    }
    // 子项菜单（页面切换）
    else if (text == "    车位申请") {
        switchToPage("车位申请");
    } else if (text == "    我的车位") {
        switchToPage("我的车位");
    } else if (text == "    我的缴费") {
        switchToPage("我的缴费");
    } else if (text == "    故障报修") {
        switchToPage("故障报修");
    } else if (text == "    维修进度查询") {
        switchToPage("维修进度查询");
    } else if (text == "    维修评价") {
        switchToPage("维修评价");
    }
}

void OwnerWindow::on_homeButton_clicked()
{
    switchToPage("回到主页");
}

void OwnerWindow::on_logoutButton_clicked()
{
    if (QMessageBox::question(this, "确认登出", "确定要退出当前账号吗？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit logoutRequested();  // 发送登出信号
        this->close();           // 关闭当前窗口
    }
}
