#ifndef ADMINHOME_H
#define ADMINHOME_H

#include <QWidget>
#include "databasemanager.h"
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class AdminHome; }
QT_END_NAMESPACE

class AdminHome : public QWidget
{
    Q_OBJECT

public:
    explicit AdminHome(int staffID, QWidget *parent = nullptr); // 构造函数增加 staffID 参数
    ~AdminHome();

private slots:
    void on_publishBtn_clicked();
    void on_modifyBtn_clicked();
    void on_deleteBtn_clicked();
    void on_noticeList_itemClicked(QListWidgetItem *item);

private:
    Ui::AdminHome *ui;
    DatabaseManager *m_dbManager;
    int m_selectedNoticeId;
    int m_staffID; // 新增成员变量存储 staffID

    bool initDatabase();
    void loadImportantInfo(); // 调整为统计 users 各类角色数量和待审批请假
    void refreshNoticeList();
    void setStyle();
};
#endif // ADMINHOME_H
