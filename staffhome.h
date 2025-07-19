#ifndef STAFFHOME_H
#define STAFFHOME_H

#include <QWidget>
#include <QVector>
#include <QVariantMap>
#include <QListWidgetItem>
#include "databasemanager.h"

namespace Ui {
class StaffHome;
}

class StaffHome : public QWidget
{
    Q_OBJECT

public:
    explicit StaffHome(int staffID, QWidget *parent = nullptr);
    ~StaffHome();

private:
    Ui::StaffHome *ui;
    DatabaseManager *m_dbManager;
    int m_staffID;
    int m_selectedNoticeId;

    bool initDatabase();
    void setStyle();
    void loadImportantInfo();
    void refreshNoticeList();

private slots:
    void on_publishBtn_clicked();
    void on_modifyBtn_clicked();
    void on_deleteBtn_clicked();
    void on_noticeList_itemClicked(QListWidgetItem *item);
};

#endif // STAFFHOME_H
