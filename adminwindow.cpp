#include "adminwindow.h"
#include "ui_adminwindow.h"
#include "personnel/querystaff.h"
#include "personnel/addstaff.h"
#include "personnel/modifystaff.h"
#include "personnel/leaveapprove.h"
#include "personnel/adminattendancequery.h"
#include "personnel/myattendancequery.h"
#include "personnel/attendancerecord.h"
#include "adminhome.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDate>
#include <QDebug>
#include <QListWidget>

AdminWindow::AdminWindow(int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AdminWindow),
    m_adminId(userId)
{
    ui->setupUi(this);
    setWindowTitle("物业管理员界面");

    initPageContainer();

    // 标记父项和子项（通过自定义数据角色存储类型：0=父项，1=子项）
    // "工作人员管理"是父项（索引0）
    QListWidgetItem* parentItem1 = ui->listWidget_menu->item(0);
    parentItem1->setData(Qt::UserRole, 0); // 0表示父项

    for (int i = 1; i <= 7; ++i) {
        QListWidgetItem* childItem1 = ui->listWidget_menu->item(i);
        childItem1->setData(Qt::UserRole, 1); // 1表示子项
    }

    QListWidgetItem* parentItem2 = ui->listWidget_menu->item(8);
    parentItem2->setData(Qt::UserRole, 0);
    QListWidgetItem* childItem1 = ui->listWidget_menu->item(9);
    childItem1->setData(Qt::UserRole, 1);

    switchToPage("回到主页");

    // 连接菜单列表的点击信号到槽函数
    connect(ui->listWidget_menu, &QListWidget::itemClicked, this, &AdminWindow::onlistWidget_menuClicked);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

// 初始化页面容器
void AdminWindow::initPageContainer()
{
    stackedWidget = new QStackedWidget(ui->rightContainer); // 右侧显示区域
    QVBoxLayout *layout = new QVBoxLayout(ui->rightContainer);
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距
    layout->addWidget(stackedWidget);
}

// 创建并添加页面
void AdminWindow::createAndAddPage(const QString &pageName, QWidget *page)
{
    qDebug() << "创建页面：" << pageName;
    page->setWindowFlags(Qt::FramelessWindowHint); // 无边框
    page->setAttribute(Qt::WA_TranslucentBackground, false);
    stackedWidget->addWidget(page);
    pageMap[pageName] = page; // 缓存页面
}

// 切换页面
void AdminWindow::switchToPage(const QString &pageName)
{
    // 页面不存在则创建（按需加载）
    if (!pageMap.contains(pageName)) {
        if (pageName == "查询工作人员") {
            createAndAddPage(pageName, new QueryStaff());
        } else if (pageName == "添加工作人员") {
            createAndAddPage(pageName, new AddStaff());
        } else if (pageName == "修改工作人员") {
            createAndAddPage(pageName, new ModifyStaff());
        } else if (pageName == "请假审核") {
            createAndAddPage(pageName, new LeaveApprove(m_adminId));
        } else if (pageName == "月度出勤查询") {
            createAndAddPage(pageName, new AdminAttendanceQuery());
        } else if (pageName == "我的月度出勤") {
            createAndAddPage(pageName, new MyAttendanceQuery(m_adminId));
        } else if (pageName == "出勤登记") {
            createAndAddPage(pageName, new AttendanceRecord(m_adminId));
        } else if (pageName == "回到主页") {
            createAndAddPage(pageName, new AdminHome(m_adminId));
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

// 系统初始化
void AdminWindow::onSystemInitClicked() {
    // 二次确认
    if (QMessageBox::question(this, "警告", "确定要初始化系统吗？所有数据将被清空（仅保留users表中role=0的记录）！",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // 临时初始化数据库（仅在当前操作中使用）
    DatabaseManager dbManager;
    QString connName = QString("SystemInit_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());
    if (!dbManager.initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", "初始化失败：" + dbManager.lastError());
        return;
    }

    // 开始事务（确保操作原子性）
    if (!dbManager.beginTransaction()) {
        QMessageBox::critical(this, "事务错误", "无法开启事务：" + dbManager.lastError());
        dbManager.closeDatabase();
        return;
    }

    try {
        // 保留users表中role=0的记录，删除其他记录
        QString deleteUsersSql = "DELETE FROM users WHERE role != 0";
        if (!dbManager.executeQuery(deleteUsersSql, {})) {
            throw QString("删除users表数据失败：" + dbManager.lastError());
        }

        // 获取数据库中所有表名（排除users表）
        QString getTablesSql = "SELECT name FROM sqlite_master WHERE type='table' AND name != 'users'";
        QVector<QVariantMap> tablesResult = dbManager.executeSelectQuery(getTablesSql, {});
        if (tablesResult.isEmpty() && dbManager.lastError().isEmpty()) {
            // 没有其他表，直接跳过
        } else if (!dbManager.lastError().isEmpty()) {
            throw QString("获取表名失败：" + dbManager.lastError());
        }

        // 清空其他表的所有数据（保留表结构）
        foreach (const QVariantMap& tableMap, tablesResult) {
            QString tableName = tableMap["name"].toString(); // 从查询结果中提取表名
            if (tableName.isEmpty()) continue;

            // 执行清空表数据的SQL
            QString clearTableSql = QString("DELETE FROM %1").arg(tableName);
            if (!dbManager.executeQuery(clearTableSql, {})) {
                throw QString("清空表%1失败：%2").arg(tableName, dbManager.lastError());
            }
        }

        // 提交事务
        if (!dbManager.commitTransaction()) {
            throw QString("提交事务失败：" + dbManager.lastError());
        }

        // 操作成功
        QMessageBox::information(this, "成功", "系统初始化完成！\n- 已保留users表中role=0的记录\n- 已清空其他表所有数据");

    } catch (const QString& errorMsg) {
        // 出错时回滚事务
        dbManager.rollbackTransaction();
        QMessageBox::critical(this, "操作失败", errorMsg);
    }

    // 关闭数据库，释放资源
    dbManager.closeDatabase();
}

//切换子项的显示/隐藏状态
void AdminWindow::onStaffManagementClicked() {
    bool isHidden = ui->listWidget_menu->item(1)->isHidden(); // 以第一个子项状态为参考
    for (int i = 1; i <= 7; ++i) {
        ui->listWidget_menu->item(i)->setHidden(!isHidden); // 取反当前状态
    }
}

void AdminWindow::onSystemSettingClicked() {
    bool isHidden = ui->listWidget_menu->item(9)->isHidden();
    ui->listWidget_menu->item(9)->setHidden(!isHidden); // 取反当前状态
}

// 菜单点击槽函数
void AdminWindow::onlistWidget_menuClicked(QListWidgetItem *item) {
    QString text = item->text();
    if (text == "    查询工作人员") {
        switchToPage("查询工作人员");
    } else if (text == "    添加工作人员") {
        switchToPage("添加工作人员");
    } else if (text == "    修改工作人员") {
        switchToPage("修改工作人员");
    } else if (text == "    请假审核") {
        switchToPage("请假审核");
    } else if (text == "    月度出勤查询") {
        switchToPage("月度出勤查询");
    } else if (text == "    我的月度出勤") {
        switchToPage("我的月度出勤");
    } else if (text == "    出勤登记") {
        switchToPage("出勤登记");
    } else if (text == "    系统初始化") {
        onSystemInitClicked();
    } else if (text == "工作人员管理") {
        onStaffManagementClicked();
    } else if (text == "系统设置") {
        onSystemSettingClicked();
    }
}

void AdminWindow::on_homeButton_clicked()
{
    switchToPage("回到主页");
}

void AdminWindow::on_logoutButton_clicked()
{
    if (QMessageBox::question(this, "确认登出", "确定要退出当前账号吗？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit logoutRequested();  // 发送登出信号
        this->close();           // 关闭当前窗口
    }
}
